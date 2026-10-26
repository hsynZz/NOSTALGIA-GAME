"""Optional conventional Cycles render denoising with Intel Open Image Denoise.

Consumes the real linear EXR render, retains a raw JPEG for comparison, never
uses image generation or adds scene content. OIDN is an external optional tool.
"""
from pathlib import Path
import subprocess

import bpy
import numpy as np


def finish_render(exr, output, raw_output, oidn, scene):
    image=bpy.data.images.load(str(exr),check_existing=False)
    scene.render.image_settings.file_format='JPEG'
    image.save_render(str(raw_output),scene=scene)
    w,h=image.size
    values=np.empty(w*h*4,dtype=np.float32)
    image.pixels.foreach_get(values)
    rgb=values.reshape(h,w,4)[:,:,:3].copy()
    input_pfm=Path(exr).with_suffix('.pfm')
    output_pfm=Path(exr).with_suffix('.denoised.pfm')
    with input_pfm.open('wb') as f:
        f.write(f'PF\n{w} {h}\n-1.0\n'.encode())
        f.write(rgb.astype('<f4').tobytes())
    subprocess.run([str(oidn),'--hdr',str(input_pfm),'-o',str(output_pfm),
                    '--quality','high','--threads','8'],check=True)
    with output_pfm.open('rb') as f:
        if f.readline().strip()!=b'PF': raise ValueError('Unexpected denoiser output')
        if tuple(map(int,f.readline().split()))!=(w,h): raise ValueError('Denoiser changed image dimensions')
        scale=float(f.readline())
        result=np.frombuffer(f.read(),dtype='<f4' if scale<0 else '>f4').reshape(h,w,3)
    if not np.isfinite(result).all(): raise ValueError('Non-finite denoised render')
    rgba=np.ones((h,w,4),dtype=np.float32)
    rgba[:,:,:3]=result*abs(scale)
    denoised=bpy.data.images.new('Denoised actual Cycles render',w,h,float_buffer=True)
    denoised.pixels.foreach_set(rgba.ravel())
    denoised.save_render(str(output),scene=scene)
    bpy.data.images.remove(denoised)
    bpy.data.images.remove(image)
