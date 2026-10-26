import * as THREE from 'three';

// All artwork is original procedural geometry/canvas art. No network assets or font downloads.
export function buildWorld(scene) {
  const colliders = [], interactables = [], doors = [], lights = [];
  const boxGeometry = new THREE.BoxGeometry(1, 1, 1);
  const sphereGeometry = new THREE.SphereGeometry(1, 12, 8);
  const cylinderGeometry = new THREE.CylinderGeometry(1, 1, 1, 10);
  const materials = new Map();
  function mat(color, roughness = .88) {
    const key = `${color}/${roughness}`;
    if (!materials.has(key)) materials.set(key, new THREE.MeshStandardMaterial({ color, roughness }));
    return materials.get(key);
  }
  const M = {
    plaster: mat('#eee3cb'), sage: mat('#a8b29a'), cream: mat('#fff0d1'), wood: mat('#916444'),
    darkwood: mat('#5e4534'), paleWood: mat('#bd9567'), green: mat('#8da06a'),
    wallBlue: mat('#c2ccd0'), rose: mat('#b8796c'), blue: mat('#80a2ae'),
    roof: mat('#b26c4e'), iron: mat('#434b43'), black: mat('#0b1014'), white: mat('#fff8e7'),
  };
  function collision(id, x, z, sx, sz, height, baseY = 0, enabled = true) {
    const c = { id, minX: x - sx / 2, maxX: x + sx / 2, minZ: z - sz / 2,
      maxZ: z + sz / 2, height, baseY, enabled };
    colliders.push(c); return c;
  }
  function box(x, y, z, sx, sy, sz, material, solid = false, parent = scene, id = 'solid') {
    const m = new THREE.Mesh(boxGeometry, material);
    m.position.set(x, y, z); m.scale.set(sx, sy, sz); parent.add(m);
    if (solid) collision(id, x, z, sx, sz, sy, y - sy / 2);
    return m;
  }
  function sphere(x, y, z, sx, sy, sz, material, parent = scene) {
    const m = new THREE.Mesh(sphereGeometry, material);
    m.position.set(x, y, z); m.scale.set(sx, sy, sz); parent.add(m); return m;
  }
  function cylinder(x, y, z, radius, height, material, parent = scene) {
    const m = new THREE.Mesh(cylinderGeometry, material);
    m.position.set(x, y, z); m.scale.set(radius, height, radius); parent.add(m); return m;
  }
  function canvasTexture(draw, width = 512, height = 256) {
    const canvas = document.createElement('canvas'); canvas.width = width; canvas.height = height;
    draw(canvas.getContext('2d'), width, height);
    const texture = new THREE.CanvasTexture(canvas); texture.colorSpace = THREE.SRGBColorSpace;
    texture.anisotropy = 2; return texture;
  }
  function label(text, x, y, z, width = 2.6, height = .65, rotation = 0, bg = '#f4edda', fg = '#39493d') {
    const texture = canvasTexture((ctx, w, h) => {
      ctx.fillStyle = bg; ctx.fillRect(0, 0, w, h);
      ctx.strokeStyle = fg; ctx.lineWidth = 5; ctx.strokeRect(8, 8, w - 16, h - 16);
      const lines = text.split('\n'); ctx.fillStyle = fg; ctx.textAlign = 'center'; ctx.textBaseline = 'middle';
      ctx.font = `600 ${Math.min(54, 155 / lines.length)}px Georgia, serif`;
      lines.forEach((line, i) => ctx.fillText(line, w / 2, h * (i + .5) / lines.length, w - 32));
    });
    const mesh = new THREE.Mesh(new THREE.PlaneGeometry(width, height), new THREE.MeshBasicMaterial({ map: texture, side: THREE.DoubleSide }));
    mesh.position.set(x, y, z); mesh.rotation.y = rotation; scene.add(mesh); return mesh;
  }
  function interactive(id, mesh, labelText) {
    mesh.userData.interaction = id;
    mesh.traverse(child => { child.userData.interaction = id; });
    interactables.push({ id, mesh, label: labelText }); return mesh;
  }

  // Soft summer sky, rendered once behind the scene with a simple two-color gradient.
  const sky = new THREE.Mesh(new THREE.SphereGeometry(350, 20, 12), new THREE.ShaderMaterial({
    side: THREE.BackSide, depthWrite: false,
    uniforms: { top: { value: new THREE.Color('#7bb6ce') }, bottom: { value: new THREE.Color('#fbecce') } },
    vertexShader: 'varying vec3 vPosition; void main(){vPosition=position;gl_Position=projectionMatrix*modelViewMatrix*vec4(position,1.0);}',
    fragmentShader: 'varying vec3 vPosition; uniform vec3 top; uniform vec3 bottom; void main(){float h=smoothstep(-.08,.65,normalize(vPosition).y);gl_FragColor=vec4(mix(bottom,top,h),1.0);}',
  }));
  sky.renderOrder = -10; scene.add(sky);
  scene.fog = new THREE.Fog('#c9d7c4', 65, 155);
  const hemi = new THREE.HemisphereLight('#e8f4ff', '#a4a084', 2.0); scene.add(hemi);
  const sun = new THREE.DirectionalLight('#fff0c7', 2.4); sun.position.set(-25, 55, -35); scene.add(sun);
  const cloudMaterial = mat('#fff9e9');
  const clouds = [];
  for (let i = 0; i < 9; i++) {
    const cloud = new THREE.Group(); cloud.position.set(-30 + i * 19, 21 + (i % 3) * 6, i % 2 ? -48 : 48);
    sphere(0, 0, 0, 5, 1.5, 2.2, cloudMaterial, cloud);
    sphere(-2.4, .4, 0, 2.4, 2.2, 2, cloudMaterial, cloud);
    sphere(1, 1, .2, 3.2, 2.8, 2.4, cloudMaterial, cloud);
    scene.add(cloud); clouds.push(cloud);
  }

  // Flat support plane is deliberate: gameplay physics uses one continuous Y=0 ground.
  box(35, -.055, 8, 170, .1, 130, mat('#9bac75'));
  box(21, -.022, 0, 80, .035, 47, mat('#b5bf8a'));
  box(31, .005, 0, 62, .015, 8, mat('#aaaca2'));
  box(80, .005, 0, 30, .015, 14, mat('#a2a799'));
  for (const z of [-4.25, 4.25]) box(32, .015, z, 58, .03, .45, mat('#d6cfb6'));
  for (let x = 8; x < 58; x += 6) box(x, .018, 0, 2.6, .01, .1, mat('#e9e1c7'));
  // Hedge boundaries frame the authored route and prevent walking around its transition.
  for (const z of [-23.5, 23.5]) box(21, 1.6, z, 80, 3.2, .4, M.green, true, scene, 'neighborhood-boundary');
  for (const x of [-19, 61]) box(x, 1.6, 0, .4, 3.2, 47, M.green, true, scene, 'neighborhood-boundary');
  for (const z of [-7.2, 7.2]) box(80, 1.6, z, 30, 3.2, .4, M.green, true, scene, 'finale-boundary');
  for (const x of [65, 95]) box(x, 1.6, 0, .4, 3.2, 14.4, M.green, true, scene, 'finale-boundary');

  const floorTexture = canvasTexture((ctx, w, h) => {
    ctx.fillStyle = '#b88c5c'; ctx.fillRect(0, 0, w, h);
    for (let y = 0; y < h; y += 32) {
      ctx.fillStyle = y % 64 ? '#bc9468' : '#b48a5d'; ctx.fillRect(0, y, w, 30);
      ctx.strokeStyle = '#9b754f'; ctx.beginPath(); ctx.moveTo((y % 96) * 3, y); ctx.lineTo((y % 96) * 3, y + 30); ctx.stroke();
    }
  });
  floorTexture.wrapS = floorTexture.wrapT = THREE.RepeatWrapping; floorTexture.repeat.set(3, 1);
  const floorMat = new THREE.MeshStandardMaterial({ map: floorTexture, roughness: 1 });
  function wall(x, z, sx, sz, material = M.plaster, height = 3.2) {
    return box(x, height / 2, z, sx, height, sz, material, true, scene, 'wall');
  }
  function endWall(x, centerZ, width, open, material = M.plaster) {
    if (!open) return wall(x, centerZ, .16, width, material);
    const sideWidth = (width - 2.4) / 2;
    wall(x, centerZ - (1.2 + sideWidth / 2), .16, sideWidth, material);
    wall(x, centerZ + (1.2 + sideWidth / 2), .16, sideWidth, material);
    box(x, 2.82, centerZ, .18, .76, 2.4, material, true, scene, 'door-lintel');
  }
  function room(x, z, sx, sz, ends = [false, false], material = M.plaster) {
    box(x, .006, z, sx, .012, sz, floorMat);
    wall(x, z - sz / 2, sx, .16, material); wall(x, z + sz / 2, sx, .16, material);
    endWall(x - sx / 2, z, sz, ends[0], material); endWall(x + sx / 2, z, sz, ends[1], material);
    box(x, 3.25, z, sx + .2, .12, sz + .2, M.cream, true, scene, 'ceiling');
  }
  // Bedroom: one real window opening, with an invisible collider matching its sill/front plane.
  box(-10, .006, 0, 16, .012, 8, floorMat);
  wall(-10, -4, 16, .16, M.wallBlue);
  wall(-15.8, 4, 4.4, .16, M.wallBlue); wall(-6, 4, 8, .16, M.wallBlue);
  box(-11.8, .45, 4, 3.6, .9, .16, M.wallBlue);
  box(-11.8, 2.85, 4, 3.6, .7, .16, M.wallBlue);
  collision('window', -11.8, 4, 3.6, .16, 3.2);
  const glass = new THREE.MeshBasicMaterial({ color: '#c8e4d8', transparent: true, opacity: .17, side: THREE.DoubleSide });
  box(-11.8, 1.7, 4.01, 3.5, 1.5, .02, glass);
  box(-11.8, 1.7, 3.96, .08, 1.7, .08, M.white); box(-11.8, 1.7, 3.96, 3.7, .08, .08, M.white);
  for (const x of [-13.65, -9.95]) box(x, 1.7, 3.98, .1, 1.8, .15, M.white);
  endWall(-18, 0, 8, false, M.wallBlue); endWall(-2, 0, 8, true, M.wallBlue);
  box(-10, 3.25, 0, 16.1, .12, 8.1, M.cream, true, scene, 'ceiling');
  room(2, 0, 8, 8, [true, true]);
  const bedroomLight = new THREE.PointLight('#ffe0ae', 7, 18, 2); bedroomLight.position.set(-10, 2.8, 0); scene.add(bedroomLight);
  const homeLight = new THREE.PointLight('#ffe6ba', 5, 12, 2); homeLight.position.set(2, 2.7, 0); scene.add(homeLight);
  // Fitted skirting and doorstep make the home feel lived-in rather than like a test room.
  for (const z of [-3.87, 3.87]) box(-6, .12, z, 23.7, .22, .1, M.cream);
  box(6.55, .009, 0, 1, .02, 2.6, M.paleWood);

  function door(id, x) {
    const group = new THREE.Group(); group.position.set(x, 0, -1.1); scene.add(group);
    box(0, 1.2, 1.1, .14, 2.4, 2.2, M.paleWood, false, group);
    box(-.085, 1.3, 1.1, .035, 1.65, 1.65, M.cream, false, group);
    sphere(-.14, 1.05, 1.92, .055, .055, .055, mat('#bc9654', .35), group);
    const c = collision(id, x, 0, .18, 2.2, 2.4);
    interactive(id, group, 'Tür öffnen / schließen'); doors.push({ id, group, collider: c });
  }
  door('door-bedroom', -2); door('door-front', 6);
  // Bed, folded blanket, original wall art, CRT and old toy shapes.
  box(-14, .28, -2.65, 3.3, .55, 1.8, M.wood, true, scene, 'bed');
  box(-14, .62, -2.65, 3.15, .25, 1.65, M.cream);
  box(-13.65, .8, -2.65, 2.2, .12, 1.67, M.sage);
  box(-15.15, .83, -2.65, .55, .18, 1.3, M.white);
  box(-15.7, .75, -2.65, .15, 1.35, 1.95, M.paleWood);
  box(-8.5, .46, 2.8, 2.1, .92, 1, M.paleWood, true, scene, 'desk');
  box(-6.2, .45, 3.1, 1.6, .9, .9, M.wood, true, scene, 'tv-cabinet');
  box(-6.2, 1.25, 3.1, 1.05, .7, .62, mat('#aaa892'));
  box(-6.2, 1.25, 2.77, .86, .53, .035, mat('#4a6b67', .25));
  box(-5.76, 1.06, 2.72, .08, .08, .04, M.iron);
  label('AUGUST\n2004', -17.89, 1.8, -1, 1.1, 1.25, Math.PI / 2, '#efe8ce', '#8e6b4d');
  label('SOMMERFERIEN', -10.8, 1.9, -3.89, 3, .7, 0, '#b9d0c2', '#405e5c');
  sphere(-11.5, .28, -3.1, .28, .28, .28, mat('#bd785b'));
  const toy = new THREE.Group(); toy.position.set(-16.8, .25, 2.2); scene.add(toy);
  sphere(0, .2, 0, .22, .3, .16, mat('#c69b6e'), toy); sphere(0, .6, 0, .22, .22, .2, mat('#c69b6e'), toy);
  sphere(-.18, .76, 0, .09, .09, .08, M.wood, toy); sphere(.18, .76, 0, .09, .09, .08, M.wood, toy);
  // Photo artwork matches the garden clue: four chairs and one house behind the table.
  const photoTexture = canvasTexture((ctx, w, h) => {
    ctx.fillStyle = '#fff9e7'; ctx.fillRect(0, 0, w, h);
    ctx.fillStyle = '#a4ccdb'; ctx.fillRect(16, 16, w - 32, h - 48);
    ctx.fillStyle = '#9ab06b'; ctx.fillRect(16, h * .55, w - 32, h * .45 - 32);
    ctx.fillStyle = '#e8c699'; ctx.fillRect(w * .42, h * .23, w * .22, h * .36);
    ctx.fillStyle = '#b97857'; ctx.beginPath(); ctx.moveTo(w * .38, h * .25); ctx.lineTo(w * .53, h * .08); ctx.lineTo(w * .68, h * .25); ctx.fill();
    ctx.fillStyle = '#fff5d6'; ctx.fillRect(w * .39, h * .67, w * .3, h * .12);
    ctx.fillStyle = '#886441'; for (const [x, y] of [[.33,.68],[.72,.68],[.52,.59],[.52,.83]]) ctx.fillRect(w*x,h*y,20,20);
    ctx.fillStyle = '#5e5445'; ctx.font = '16px Georgia'; ctx.fillText('17. August 2004', 22, h - 12);
  });
  const photo = box(-8.5, .985, 2.6, .68, .07, .45, new THREE.MeshStandardMaterial({ map: photoTexture, roughness: 1 }));
  interactive('photo', photo, 'Geburtstagsfoto ansehen');
  label('17. AUGUST 2004', -8.5, 1.22, 3.4, 1.35, .25, Math.PI, '#eee4cb', '#625849').userData.interaction = 'photo';

  // Living room, with a clear route from bedroom to the front door.
  box(1.2, .45, 2.7, 3.1, .9, 1.1, M.sage, true, scene, 'sofa');
  box(1.2, 1.04, 3.12, 3.1, .7, .3, M.sage);
  for (const x of [-.25, 2.65]) box(x, .75, 2.7, .24, .7, 1.2, M.sage);
  box(2.6, .36, -2.4, 1.7, .72, 1, M.paleWood, true, scene, 'low-table');
  box(2.4, .77, -2.5, .5, .08, .33, mat('#80939a'));
  label('ZU HAUSE', 3, 1.75, -3.89, 2, .65, 0, '#e9dec5', '#6d7964');

  function house(x, z, number, color) {
    const group = new THREE.Group(); scene.add(group);
    box(x, 2.5, z, 8, 5, 8, mat(color), true, group, 'neighbor-house');
    const roof = new THREE.Mesh(new THREE.ConeGeometry(6.2, 2.1, 4), M.roof); roof.position.set(x, 6.05, z); roof.rotation.y = Math.PI / 4; group.add(roof);
    const side = z > 0 ? -1 : 1, front = z + side * 4.04;
    for (const dx of [-2.35, 2.35]) {
      box(x + dx, 2.6, front, 1.55, 1.65, .05, mat('#91afb0'), false, group);
      box(x + dx, 2.6, front + side * .025, .06, 1.7, .06, M.cream, false, group);
      box(x + dx, 2.6, front + side * .026, 1.6, .06, .06, M.cream, false, group);
    }
    box(x, 1.2, front, 1.5, 2.4, .1, M.sage, false, group);
    label(String(number), x + 1.05, 1.6, front + side * .04, .45, .45, side > 0 ? 0 : Math.PI);
    box(x, .01, z + side * 5.9, 1.55, .02, 3.7, mat('#d8cfb4'));
    return group;
  }
  house(12, -15.5, 10, '#ded1b1'); house(12, 15.5, 11, '#d7bda2');
  house(32, -15.5, 12, '#d0d0b6'); house(32, 15.5, 13, '#c8cec0');
  house(50, -15.5, 14, '#e0cbb0'); house(50, 15.5, 15, '#d7c3aa');
  const changedNumber = label('13 ½', 44, 1.6, 4.8, 1, .55, -Math.PI / 2); changedNumber.visible = false;
  const streetName = label('SOMMERWEG', 9, 2.2, -5.5, 2.6, .6, -Math.PI / 2, '#6e8c75', '#fff5da');
  cylinder(9, 1, -5.5, .055, 2, M.iron);
  cylinder(55.7, 1.15, -2.8, .055, 2.3, M.iron);
  label('STOP', 55.7, 2.35, -2.8, 1.25, 1.2, -Math.PI / 2, '#b65e4b', '#fff3d9');
  label('Wenn die Laternen angehen,\nkomm nach Hause.', 8.3, 1.15, 4.95, 3.5, .9, Math.PI / 2, '#e4ddc6', '#576251');

  // Birthday garden; no fetch quest. The displaced fourth chair is a visible memory discrepancy.
  box(22, .018, 11, 11, .03, 9, mat('#b7bf85'));
  const cloth = canvasTexture((ctx, w, h) => {
    ctx.fillStyle = '#fff0d1'; ctx.fillRect(0,0,w,h); ctx.fillStyle = '#ce837266';
    for (let x=0; x<w; x+=48) ctx.fillRect(x,0,24,h);
    for (let y=0; y<h; y+=48) ctx.fillRect(0,y,w,24);
  });
  const clothMat = new THREE.MeshStandardMaterial({ map: cloth, roughness: 1 });
  const table = box(24, .7, 11, 1.65, .15, 2.2, clothMat, true, scene, 'birthday-table');
  collision('birthday-table-volume', 24, 11, 1.65, 2.2, .8);
  for (const x of [23.4, 24.6]) for (const z of [10.2, 11.8]) box(x, .35, z, .12, .7, .12, M.wood);
  cylinder(24, .88, 11, .38, .22, M.cream); cylinder(24, 1.01, 11, .29, .05, mat('#b66e66'));
  for (const x of [23.84, 24.16]) for (const z of [10.85, 11.15]) cylinder(x, 1.15, z, .025, .25, mat('#e8c76b'));
  interactive('birthday', table, 'Geburtstagserinnerung ordnen');
  // The cake also accepts interaction, so the central visible target never masks the table.
  for (const object of scene.children) {
    if (Math.abs(object.position.x-24)<.5 && Math.abs(object.position.z-11)<.5 && object.position.y >= .85 && object.position.y < 1.4)
      object.userData.interaction = 'birthday';
  }
  function chair(x, z, angle = 0) {
    const group = new THREE.Group(); group.position.set(x, 0, z); group.rotation.y = angle; scene.add(group);
    box(0, .45, 0, .65, .12, .65, M.paleWood, false, group);
    box(0, .83, .28, .65, .7, .1, M.paleWood, false, group);
    for (const dx of [-.25,.25]) for (const dz of [-.25,.25]) box(dx, .2, dz, .08, .4, .08, M.wood, false, group);
    return group;
  }
  chair(22.3, 11, -Math.PI / 2); collision('chair', 22.3, 11, .7, .7, 1.15);
  chair(24, 8.8, Math.PI); collision('chair', 24, 8.8, .7, .7, 1.15);
  chair(24, 13.2, 0); collision('chair', 24, 13.2, .7, .7, 1.15);
  const birthdayChair = chair(27.2, 14.5, .5); // Moving memory prop deliberately has no collision.
  const balloonColors = ['#c97b6b', '#d7b466', '#8ca9b7'];
  for (let i=0; i<3; i++) {
    const x=25.6+i*.4, z=13.6+(i%2)*.45;
    cylinder(x, 1.2, z, .009, 2.4, M.cream); sphere(x, 2.55+i*.18, z, .3, .4, .3, mat(balloonColors[i]));
  }
  label('GEBURTSTAGSGARTEN', 17.8, 1.65, 6.3, 3.6, .6, -Math.PI / 2, '#ede3c3', '#637453');
  const compare = box(20.5, .9, 9, .3, 1.8, .75, M.paleWood, true, scene, 'comparison-post');
  interactive('compare', compare, 'Foto mit dem leeren Grundstück vergleichen');
  const compareSign = label('Auf dem Foto steht\nhier ein Haus.', 20.3, 1.35, 9, 2, .7, -Math.PI / 2);
  compareSign.userData.interaction = 'compare';
  const memoryHouse = new THREE.Group(); memoryHouse.position.set(24.5, 0, 20.5); scene.add(memoryHouse);
  box(0, 2, 0, 4.5, 4, 4.5, mat('#e6d6b5'), false, memoryHouse);
  const memoryRoof = new THREE.Mesh(new THREE.ConeGeometry(3.5, 1.5, 4), M.roof);
  memoryRoof.position.y=4.75; memoryRoof.rotation.y=Math.PI/4; memoryHouse.add(memoryRoof);
  box(-1, 2.2, -2.28, 1.1, 1.3, .04, M.blue, false, memoryHouse); box(1, 1.2, -2.28, 1.3, 2.4, .04, M.sage, false, memoryHouse);
  memoryHouse.visible=false;

  // Curated impossible interior, connected by explicit portals to the same house facade.
  room(-10, 32, 22, 10, [false,false]);
  for (const x of [-13.5,-6.5,.5]) {
    for (const z of [28.5,35.5]) box(x,1.5,z,.18,3,3,M.cream,true,scene,'interior-threshold');
    box(x,2.9,32,.18,.35,4,M.paleWood);
  }
  const interiorLight = new THREE.PointLight('#f1d9b7', 9, 24, 2); interiorLight.position.set(-10,2.8,32); scene.add(interiorLight);
  label('Derselbe Flur.\nViel zu weit.', -8,1.8,27.1,3,.8,0,'#d3d8c7','#616d5e');
  sphere(-4,2.1,34,.28,.4,.28,mat('#b46d62')); cylinder(-4,1.1,34,.01,2.1,M.cream);
  function portal(id,x,z,axis,labelText,material=M.sage) {
    const object = axis==='x' ? box(x,1.2,z,.15,2.4,1.4,material,true,scene,id) : box(x,1.2,z,1.4,2.4,.15,material,true,scene,id);
    interactive(id,object,labelText); return object;
  }
  portal('house-entry',12,-11.28,'z','Haus 10 betreten');
  portal('house-return',-19.5,32,'x','Zur vertrauten Straße zurück');
  label('ZURÜCK',-19.39,1.8,32,1.15,.38,Math.PI/2).userData.interaction='house-return';

  // Lamp chain: warm practical lights and one visibly dark head at the new street's end.
  const glowingLampMat = new THREE.MeshStandardMaterial({color:'#f7d990',emissive:'#f6c66d',emissiveIntensity:0});
  for (const x of [9,24,39,54,85]) {
    cylinder(x,2.2,4.2,.08,4.4,M.iron); collision('lamp-post',x,4.2,.18,.18,4.4);
    box(x,4.4,4.2,.8,.15,.8,M.iron);
    sphere(x,4.23,4.2,.27,.25,.27,x===85?M.iron:glowingLampMat);
    if(x!==85){ const light=new THREE.PointLight('#ffcf78',0,10,2);light.position.set(x,3.9,4.2);scene.add(light);lights.push(light); }
  }
  const lampPlaque=box(85,1.05,3.3,.65,1.3,.15,M.iron,true,scene,'lamp-plaque');
  interactive('lamp',lampPlaque,'Die dunkle Laterne untersuchen');
  label('Wenn alle leuchten ...',84.9,1.45,3.18,2,.45,-Math.PI/2).userData.interaction='lamp';
  // Small exterior shell, long separate interior: the final spatial anomaly is walkable.
  box(89,1.2,4.5,3,2.4,3.2,mat('#b6c0a0'),true,scene,'playhouse-shell');
  const playRoof=new THREE.Mesh(new THREE.ConeGeometry(2.55,1.4,4),M.roof);playRoof.position.set(89,3.1,4.5);playRoof.rotation.y=Math.PI/4;scene.add(playRoof);
  portal('playhouse-entry',87.4,4.5,'x','Spielhaus betreten',M.cream);
  label('SPIELHAUS',87.3,2.35,4.5,1.5,.35,-Math.PI/2).userData.interaction='playhouse-entry';
  room(80,30,22,4,[false,false],mat('#ced0b5'));
  for(const x of [75,80,85]){
    box(x,1.5,28.35,.15,3,.7,M.cream,true,scene,'playhouse-threshold');
    box(x,1.5,31.65,.15,3,.7,M.cream,true,scene,'playhouse-threshold');
    box(x,2.9,30,.15,.35,2.6,M.paleWood);
  }
  const corridorLight=new THREE.PointLight('#edd69e',12,27,2);corridorLight.position.set(80,2.8,30);scene.add(corridorLight);
  portal('playhouse-return',69.4,30,'x','Zur dunklen Laterne zurück');
  portal('exit',90,30,'x','Die Tür am Ende öffnen',M.paleWood);
  label('DRAUSSEN WAR ES KLEINER.',80,1.6,28.1,3.2,.55,0);
  portal('street-return',69,-5,'x','Zur Nachbarschaft zurück');
  label('SOMMERWEG\nZURÜCK',69.2,2.6,-5,1.8,.65,Math.PI/2).userData.interaction='street-return';

  // Original elongated silhouette. The main game controls visibility and the encounter state.
  const homecomer=new THREE.Group();homecomer.position.set(35,0,8.5);scene.add(homecomer);
  sphere(0,2,0,.29,1.12,.26,M.black,homecomer);sphere(0,3.4,0,.23,.31,.23,M.black,homecomer);
  for(const side of [-1,1]){
    sphere(side*.33,1.8,0,.075,1.23,.075,M.black,homecomer);
    sphere(side*.14,.75,0,.11,.91,.12,M.black,homecomer);
  }
  homecomer.visible=false;
  // Batch fixed scene-level boxes by material. Dynamic/interactive groups stay separate for raycasts.
  const batches = new Map();
  for (const object of [...scene.children]) {
    if (object.isMesh && object.geometry === boxGeometry && !object.userData.interaction) {
      if (!batches.has(object.material)) batches.set(object.material, []);
      batches.get(object.material).push(object);
    }
  }
  for (const [material, objects] of batches) {
    if (objects.length < 2) continue;
    const instances = new THREE.InstancedMesh(boxGeometry, material, objects.length);
    objects.forEach((object, i) => { object.updateMatrix(); instances.setMatrixAt(i, object.matrix); scene.remove(object); });
    instances.computeBoundingSphere(); scene.add(instances);
  }
  const dayTop=new THREE.Color('#7bb6ce'), nightTop=new THREE.Color('#1b2741');
  const dayBottom=new THREE.Color('#fbecce'), nightBottom=new THREE.Color('#766f88');
  const dayFog=new THREE.Color('#c9d7c4'), nightFog=new THREE.Color('#70758b');
  let dusk=0;
  function update(state,time=0){
    const restored=Number(state.stage)>=3;
    const night=Number(state.minute)>=1200||Number(state.minute)<360;
    dusk+=(Number(night)-dusk)*.025;
    sky.material.uniforms.top.value.copy(dayTop).lerp(nightTop,dusk);
    sky.material.uniforms.bottom.value.copy(dayBottom).lerp(nightBottom,dusk);
    scene.fog.color.copy(dayFog).lerp(nightFog,dusk);
    hemi.intensity=2-dusk*1.15;sun.intensity=2.4-dusk*2.0;
    sun.color.set(night?'#a9bbdf':'#fff0c7');
    for(const light of lights) light.intensity=dusk*12;
    glowingLampMat.emissiveIntensity=dusk*1.8;
    changedNumber.visible=Number(state.loops)>0;
    memoryHouse.visible=restored;
    birthdayChair.position.set(restored?25.7:27.2,0,restored?11:14.5);
    birthdayChair.rotation.y=restored?Math.PI/2:.5;
    for(let i=0;i<clouds.length;i++)clouds[i].position.x=-30+i*19+Math.sin(time*.008+i)*2;
  }
  return {colliders,interactables,doors,homecomer,memoryHouse,birthdayChair,lights,sun,sky,update};
}
