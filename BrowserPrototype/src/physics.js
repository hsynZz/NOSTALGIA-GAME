export class Player {
  constructor(){this.reset();}
  reset(){this.x=-12;this.y=0;this.z=0;this.yaw=-2.15;this.pitch=-.12;this.vy=0;this.grounded=true;this.crouched=false;this.jumpHeld=false;this.distance=0;}
  get height(){return this.crouched?1.05:1.76;}
  get eye(){return this.crouched?.9:1.58;}
  free(x,z,y,height,boxes){
    return !boxes.some(b=>{
      if(b.enabled===false || y>=b.height-.005 || y+height<=(b.baseY??0)+.005)return false;
      const dx=x-Math.max(b.minX,Math.min(x,b.maxX)),dz=z-Math.max(b.minZ,Math.min(z,b.maxZ));
      return dx*dx+dz*dz<.32*.32;
    });
  }
  step(dt,input,boxes){
    dt=Math.max(0,Math.min(dt,.05));
    const wantsCrouch=!!input.crouch;
    if(wantsCrouch)this.crouched=true;
    else if(this.free(this.x,this.z,this.y,1.76,boxes))this.crouched=false;
    if(input.jump&&!this.jumpHeld&&this.grounded&&!this.crouched){this.vy=4.2;this.grounded=false;}
    this.jumpHeld=!!input.jump;
    let f=input.forward??0,r=input.right??0;
    const length=Math.hypot(f,r);if(length>1){f/=length;r/=length;}
    const speed=this.crouched?1.5:input.sprint?5.2:3.2;
    const vx=(-Math.sin(this.yaw)*f+Math.cos(this.yaw)*r)*speed;
    const vz=(-Math.cos(this.yaw)*f-Math.sin(this.yaw)*r)*speed;
    const steps=Math.max(1,Math.ceil(dt/(1/90))),delta=dt/steps;
    const beforeX=this.x,beforeZ=this.z;
    for(let i=0;i<steps;i++){
      if(this.free(this.x+vx*delta,this.z,this.y,this.height,boxes))this.x+=vx*delta;
      if(this.free(this.x,this.z+vz*delta,this.y,this.height,boxes))this.z+=vz*delta;
      const oldY=this.y;this.vy-=9.81*delta;let nextY=this.y+this.vy*delta;let floor=0;
      for(const b of boxes){
        if(b.enabled===false)continue;
        const dx=this.x-Math.max(b.minX,Math.min(this.x,b.maxX)),dz=this.z-Math.max(b.minZ,Math.min(this.z,b.maxZ));
        if(dx*dx+dz*dz>=.32*.32)continue;
        if(this.vy<=0&&oldY>=b.height-.01&&nextY<=b.height)floor=Math.max(floor,b.height);
        const ceiling=b.baseY??0;
        if(this.vy>0&&oldY+this.height<=ceiling+.01&&nextY+this.height>=ceiling){nextY=ceiling-this.height;this.vy=0;}
      }
      if(nextY<=floor){this.y=floor;this.vy=0;this.grounded=true;}else{this.y=nextY;this.grounded=false;}
    }
    const moved=Math.hypot(this.x-beforeX,this.z-beforeZ);this.distance+=moved;return dt>0?moved/dt:0;
  }
  teleport(x,z,y=0){this.x=x;this.z=z;this.y=y;this.vy=0;this.grounded=true;}
}
