import * as THREE from 'three';
import {buildWorld} from './world.js';
import {Player} from './physics.js';
import {AudioDirector} from './audio.js';
import * as Core from './core.js';

const $=id=>document.getElementById(id),canvas=$('game');
const mobile=matchMedia('(pointer: coarse)').matches;
const testing=new URLSearchParams(location.search).has('test');
let renderer;
try{renderer=new THREE.WebGLRenderer({canvas,antialias:false,powerPreference:'low-power'});}
catch(error){$('error').hidden=false;$('error-detail').textContent=error.message;throw error;}
renderer.outputColorSpace=THREE.SRGBColorSpace;
renderer.toneMapping=THREE.ACESFilmicToneMapping;renderer.toneMappingExposure=1.15;
const scene=new THREE.Scene(),camera=new THREE.PerspectiveCamera(75,innerWidth/innerHeight,.05,500);
camera.rotation.order='YXZ';
const world=buildWorld(scene),player=new Player(),audio=new AudioDirector();
// World authoring uses physical height; physics uses absolute top elevation.
world.colliders.forEach(b=>b.height+=(b.baseY??0));
let state=Core.createState(),running=false,started=false,photoOpen=false,muted=false;
let contextDistrict=0,dwell=0,transitionUntil=0,focus=null,enemy=null,enemyDistance=0;
let qualityLow=true,toastUntil=0,lastStamp=performance.now(),renderAccumulator=0,dragging=false;
let suppressUnlock=false,lookPointer=null,lastLook=null;
const keys=new Set(),touch={forward:0,right:0,jump:false,crouch:false,sprint:false};
const ray=new THREE.Raycaster(),center=new THREE.Vector2(0,0),temp=new THREE.Vector3();
const STORAGE='nostalgia.browser.01';
const interactions=new Map(world.interactables.map(i=>[i.id,i]));

function resize(){renderer.setPixelRatio(qualityLow?Math.min(devicePixelRatio,1,1280/innerWidth,720/innerHeight):Math.min(devicePixelRatio,1.75));renderer.setSize(innerWidth,innerHeight);camera.aspect=innerWidth/innerHeight;camera.updateProjectionMatrix();}
addEventListener('resize',resize);resize();
function message(text,seconds=5){$('toast').textContent=text;$('toast').style.opacity=1;toastUntil=performance.now()+seconds*1000;}
function setCamera(){camera.position.set(player.x,player.y+player.eye,player.z);camera.rotation.set(player.pitch,player.yaw,0,'YXZ');camera.updateMatrixWorld();}
function visible(object){for(let o=object;o;o=o.parent)if(!o.visible)return false;return true;}
function cast(){
  ray.setFromCamera(center,camera);ray.far=2.65;
  const hits=ray.intersectObjects(scene.children,true).filter(h=>visible(h.object));
  for(const hit of hits){
    let o=hit.object;while(o&&!o.userData.interaction)o=o.parent;
    if(o&&interactions.has(o.userData.interaction))return interactions.get(o.userData.interaction);
    // The nearest solid surface occludes objects behind it, even when the target is in reach.
    if(hit.object.isMesh&&!hit.object.userData.ignoreInteractionRay)return null;
  }
  return null;
}
function syncDoors(){
  world.doors.forEach((d,index)=>{const opened=!!(state.openDoors&(1<<index));d.group.rotation.y=opened?Math.PI/2:0;d.collider.enabled=!opened;});
}
function doorsStep(dt){world.doors.forEach((d,index)=>{
  const opened=!!(state.openDoors&(1<<index)),target=opened?Math.PI/2:0;
  let angle=d.group.rotation.y+Math.sign(target-d.group.rotation.y)*Math.min(Math.abs(target-d.group.rotation.y),dt*1.75);
  const occupied=!player.free(player.x,player.z,player.y,player.height,[{...d.collider,enabled:true}]);
  // Never enable a closed collider around a player who stepped into the moving doorway.
  if(!opened&&angle<.2&&occupied)angle=.2;
  d.group.rotation.y=angle;d.collider.enabled=angle<.18&&!occupied;
});}
function door(id){
  const index=id==='door-bedroom'?0:1,d=world.doors[index];
  if(state.openDoors&(1<<index)){
    if(!player.free(player.x,player.z,player.y,player.height,[{...d.collider,enabled:true}])){message('Tritt einen Schritt aus dem Türbereich.');return;}
  }
  state.openDoors^=1<<index;
}
function releaseMouse(){suppressUnlock=true;document.exitPointerLock?.();setTimeout(()=>suppressUnlock=false,200);}
function captureMouse(){if(mobile)return;try{const promise=canvas.requestPointerLock?.();promise?.catch(()=>message('Maus frei: linke Maustaste halten und ziehen.'));}catch{message('Maus frei: linke Maustaste halten und ziehen.');}}
function clearInput(){keys.clear();touch.forward=touch.right=0;touch.jump=touch.sprint=touch.crouch=false;}
function pause(){if(!started||state.stage===6||photoOpen)return;running=false;audio.stop();$('subtitle').textContent='';clearInput();releaseMouse();$('menu').hidden=false;$('start').firstChild.textContent='Zurück in den Sommer ';$('hud').hidden=true;$('touch').hidden=true;}
async function start(){
  await audio.unlock();audio.setMuted(muted);running=true;$('menu').hidden=true;$('hud').hidden=false;
  $('complete').hidden=true;$('touch').hidden=!mobile;$('hud').classList.toggle('touch-mode',mobile);
  captureMouse();if(!started){started=true;message('Ich kenne dieses Zimmer. Auf dem Schreibtisch liegt ein Foto.',7);}
}
function setPhoto(open){
  photoOpen=open;$('photo').hidden=!open;clearInput();
  if(open){$('photo-note').textContent=state.stage<2?'Wie sieht dieser Garten heute aus?':state.stage<3?'Ein Stuhl steht abseits. Das Haus hinter dem Tisch fehlt.':'Die Anordnung stimmt wieder. Und das Haus ist zurück.';releaseMouse();}
  else if(running)captureMouse();
}
function teleport(x,z,yaw=player.yaw){player.teleport(x,z);player.yaw=yaw;transitionUntil=state.elapsed+3;clearInput();setCamera();$('fade').style.opacity='.85';setTimeout(()=>$('fade').style.opacity='0',120);}
function finish(){running=false;audio.stop();clearInput();releaseMouse();$('hud').hidden=true;$('touch').hidden=true;$('complete').hidden=false;}
function interact(){
  if(!running||photoOpen)return;
  setCamera();const target=cast();if(!target)return;
  const id=target.id;
  if(id.startsWith('door-')){door(id);return;}
  const action={photo:'photo',compare:'compare',birthday:'birthday',lamp:'lamp',exit:'exit'}[id];
  if(action){
    const changed=Core.apply(state,action);
    if(changed){
      if(id==='photo')setPhoto(true);
      else if(id==='compare')message('Auf dem Foto steht ein Haus. Hier ist nur eine leere Stelle.');
      else if(id==='birthday'){message('Vier Stühle. So war es. Die Laternen gehen an – bis auf eine.',7);audio.stop();}
      else if(id==='lamp')message('Diese Laterne bleibt dunkel. Dahinter steht ein kleines Spielhaus.');
      else if(id==='exit')finish();
    }else if(id==='photo'&&state.stage>=1)setPhoto(true);else message(Core.hint(state.stage));
    world.update(state,state.elapsed);return;
  }
  if(state.elapsed<transitionUntil){message('Einen Moment – der Weg findet sich.');return;}
  if(id==='house-entry')teleport(-17,32,-Math.PI/2);
  else if(id==='house-return')teleport(12,-9,Math.PI);
  else if(id==='street-return')teleport(53,0,Math.PI/2);
  else if(id==='playhouse-return')teleport(85.5,4.5,Math.PI/2);
  else if(id==='playhouse-entry'){
    if(state.stage<5)message('Erst die dunkle Laterne untersuchen.');
    else{teleport(71.5,30,-Math.PI/2);message('Das Spielhaus war von außen viel kleiner.',6);}
  }
}
function save(){
  if(!started||!player.grounded||player.crouched||player.y>.01||!player.free(player.x,player.z,0,1.76,world.colliders)){message('Zum Speichern bitte aufrichten und vom Möbelstück auf den Boden treten.');return;}
  if(world.doors.some((d,index)=>!(state.openDoors&(1<<index))&&!player.free(player.x,player.z,0,1.76,[{...d.collider,enabled:true}]))){message('Zum Speichern bitte aus dem Türbereich treten.');return;}
  try{localStorage.setItem(STORAGE,Core.serializeSnapshot(state,player));$('continue').hidden=false;message('Erinnerung gespeichert – in diesem Browser.');}
  catch{message('Hier konnte nicht gespeichert werden. Bitte außerhalb von Übergängen versuchen oder Browserspeicher erlauben.');}
}
async function load(){
  let snapshot;try{snapshot=Core.parseSnapshot(localStorage.getItem(STORAGE));}catch{}
  if(!snapshot){message('Kein gültiger Browser-Spielstand vorhanden.');return;}
  const old=state;state=snapshot.state;syncDoors();
  if(!player.free(snapshot.player.x,snapshot.player.z,0,1.76,world.colliders)){state=old;syncDoors();message('Gespeicherte Position ist blockiert.');return;}
  audio.stop();enemy=null;world.homecomer.visible=false;world.homecomer.position.set(35,0,8.5);photoOpen=false;$('photo').hidden=true;
  player.reset();player.teleport(snapshot.player.x,snapshot.player.z);player.yaw=snapshot.player.yaw;player.pitch=snapshot.player.pitch;
  transitionUntil=state.elapsed+3;dwell=0;clearInput();world.update(state,state.elapsed);started=true;
  if(state.stage===6){finish();return;}await start();message('Erinnerung geladen.');
}
function enemyVisible(){
  const target=world.homecomer.position.clone().add(new THREE.Vector3(0,2.3,0));
  const distance=target.distanceTo(camera.position);if(distance<6||distance>22)return false;
  const screen=target.clone().project(camera);if(Math.abs(screen.x)>.85||Math.abs(screen.y)>.85||screen.z>1)return false;
  const direction=target.clone().sub(camera.position).normalize();
  ray.set(camera.position,direction);ray.far=distance-.3;
  return !ray.intersectObjects(scene.children,true).some(h=>visible(h.object)&&h.object.isMesh);
}
function updateEnemy(dt,safe){
  if(!enemy)return;
  enemy.time+=dt;
  if(safe||enemy.time>=11){enemy=null;world.homecomer.visible=false;return;}
  if(enemy.time>=9){
    temp.set(world.homecomer.position.x-player.x,0,world.homecomer.position.z-player.z).normalize();
    world.homecomer.position.addScaledVector(temp,dt*.8);
  }else if(enemy.time>=2&&enemy.travel<1.5){
    const to=world.homecomer.position.clone().add(new THREE.Vector3(0,2.3,0)).sub(camera.position).normalize();
    const look=camera.getWorldDirection(new THREE.Vector3()).dot(to);
    const distance=Math.hypot(world.homecomer.position.x-player.x,world.homecomer.position.z-player.z);
    if(look<.6&&distance>4.5){
      temp.set(player.x-world.homecomer.position.x,0,player.z-world.homecomer.position.z).normalize();
      const amount=Math.min(dt*.6,1.5-enemy.travel,distance-4.5);
      const nx=world.homecomer.position.x+temp.x*amount,nz=world.homecomer.position.z+temp.z*amount;
      if(player.free(nx,nz,0,3.6,world.colliders)){world.homecomer.position.x=nx;world.homecomer.position.z=nz;enemy.travel+=amount;}
    }
  }
}
function tick(dt){
  if(running&&state.stage!==6){
    const input=photoOpen?{}:{forward:(keys.has('KeyW')||keys.has('ArrowUp')?1:0)-(keys.has('KeyS')||keys.has('ArrowDown')?1:0)+touch.forward,
      right:(keys.has('KeyD')||keys.has('ArrowRight')?1:0)-(keys.has('KeyA')||keys.has('ArrowLeft')?1:0)+touch.right,
      sprint:keys.has('ShiftLeft')||keys.has('ShiftRight')||touch.sprint,crouch:keys.has('ControlLeft')||keys.has('KeyC')||touch.crouch,jump:keys.has('Space')||touch.jump};
    const speed=player.step(dt,input,world.colliders);doorsStep(dt);Core.advance(state,dt);setCamera();
    if(Math.abs(player.z)<4.7&&player.x>=56&&player.x<=58&&state.elapsed>=transitionUntil&&Core.traverseLoop(state)){
      teleport(state.stage>=4?72:9.5,Math.max(-2.5,Math.min(2.5,player.z)));
      message(state.stage>=4?'Der Weg führt weiter. Eine Laterne bleibt dunkel.':'Wieder dieselbe Straße. Eine Hausnummer war vorher nicht da.',6);
    }
    const district=Core.districtAt(player.x,player.z),safe=district===0||district===4;
    dwell=district===contextDistrict?dwell+dt:0;contextDistrict=district;
    updateEnemy(dt,safe);audio.update(player,district);
    const gardenDistance=Math.hypot(player.x-24,player.z-12.5);
    const available=[!photoOpen&&gardenDistance>=3&&gardenDistance<11&&!muted,
      !photoOpen&&!enemy&&enemyVisible(),!photoOpen&&!muted];
    const plan=Core.planEvent(state,{district,safeZone:safe,threatened:!!enemy,dwell,speed:speed*100,available});
    if(plan.id!==3){
      let delivered=false;
      if(plan.id===0){delivered=audio.playGarden();if(delivered){$('subtitle').textContent='[Kinderlachen aus dem Garten]';setTimeout(()=>$('subtitle').textContent='',5000);}}
      if(plan.id===1){world.homecomer.visible=true;enemy={time:0,travel:0};delivered=true;message('Am Straßenrand steht jemand. Viel zu still.',4);}
      if(plan.id===2){delivered=audio.knock();if(delivered){$('subtitle').textContent='[Klopfen am Fenster]';setTimeout(()=>$('subtitle').textContent='',2200);}}
      Core.commitEvent(state,plan,delivered);
    }
    focus=photoOpen?null:cast();$('prompt').textContent=focus?`E · ${focus.label}`:'';
  }
  setCamera();world.update(state,state.elapsed);
  $('objective-text').textContent=Core.objective(state.stage);
  const minutes=Math.floor(state.minute);$('time').textContent=`${String(Math.floor(minutes/60)).padStart(2,'0')}:${String(minutes%60).padStart(2,'0')}`;
  if(performance.now()>toastUntil)$('toast').style.opacity=0;
}
function frame(stamp){requestAnimationFrame(frame);const dt=Math.min(.05,(stamp-lastStamp)/1000);lastStamp=stamp;renderAccumulator+=dt;
  if(document.hidden)return;tick(dt);
  if(renderAccumulator>1/30){renderer.render(scene,camera);renderAccumulator=0;}
}
function look(dx,dy){if(!running||photoOpen)return;player.yaw-=dx*.0023;player.pitch=THREE.MathUtils.clamp(player.pitch-dy*.0023,-1.48,1.48);}
addEventListener('keydown',e=>{
  if((running&&['KeyW','KeyA','KeyS','KeyD','KeyC','KeyE','KeyJ','KeyH'].includes(e.code))||['Space','Tab','ControlLeft','ArrowUp','ArrowDown','ArrowLeft','ArrowRight','F5','F9'].includes(e.code))e.preventDefault();
  if(e.repeat)return;
  if(e.code==='Escape'){if(photoOpen)setPhoto(false);else pause();return;}
  if(e.code==='KeyJ'&&running&&state.stage>=1){setPhoto(!photoOpen);return;}
  if(e.code==='F5'){save();return;}if(e.code==='F9'){void load();return;}
  if(e.code==='KeyH'&&running)message(Core.hint(state.stage),8);
  if(e.code==='KeyE')interact();keys.add(e.code);
  if(e.code==='Tab')$('watch').classList.toggle('expanded');
});
addEventListener('keyup',e=>keys.delete(e.code));addEventListener('blur',()=>{clearInput();if(running)pause();});
addEventListener('pointerlockchange',()=>{if(!document.pointerLockElement&&!suppressUnlock&&running&&!photoOpen)pause();});
canvas.addEventListener('pointerdown',e=>{if(mobile){lookPointer=e.pointerId;lastLook={x:e.clientX,y:e.clientY};canvas.setPointerCapture(e.pointerId);}else{dragging=true;if(running&&!photoOpen&&!document.pointerLockElement)captureMouse();}});
addEventListener('pointerup',e=>{dragging=false;if(e.pointerId===lookPointer){lookPointer=null;lastLook=null;}});
addEventListener('pointermove',e=>{
  if(mobile&&e.pointerId===lookPointer&&lastLook){look((e.clientX-lastLook.x)*1.6,(e.clientY-lastLook.y)*1.6);lastLook={x:e.clientX,y:e.clientY};}
  else if(!mobile&&(document.pointerLockElement===canvas||dragging))look(e.movementX,e.movementY);
});
$('start').onclick=()=>void start();$('continue').onclick=()=>void load();$('pause').onclick=pause;
$('hint').onclick=()=>message(Core.hint(state.stage),8);$('save').onclick=save;$('load').onclick=()=>void load();
$('memory').onclick=()=>{if(state.stage>=1)setPhoto(!photoOpen);else message(Core.hint(state.stage));};
$('close-photo').onclick=()=>setPhoto(false);$('watch').onclick=()=>$('watch').classList.toggle('expanded');
$('sound').onclick=()=>{muted=!muted;audio.setMuted(muted);$('sound').textContent=`Ton: ${muted?'aus':'an'}`;};
$('quality').onclick=()=>{qualityLow=!qualityLow;resize();$('quality').textContent=`Grafik: ${qualityLow?'sparsam':'fein'}`;};
$('fullscreen').onclick=()=>{const p=document.fullscreenElement?document.exitFullscreen():document.documentElement.requestFullscreen?.();p?.catch(()=>message('Vollbild ist in diesem Browser nicht verfügbar.'));};
$('restart').onclick=()=>{state=Core.createState();player.reset();enemy=null;world.homecomer.position.set(35,0,8.5);world.homecomer.visible=false;syncDoors();photoOpen=false;started=false;transitionUntil=0;void start();};
$('end-load').onclick=()=>void load();
const stick=$('stick');let stickPointer=null;
function moveStick(e){const r=stick.getBoundingClientRect(),dx=(e.clientX-r.left-r.width/2)/40,dz=(e.clientY-r.top-r.height/2)/40,len=Math.max(1,Math.hypot(dx,dz));touch.right=dx/len;touch.forward=-dz/len;stick.firstChild.style.transform=`translate(${dx/len*30}px,${dz/len*30}px)`;}
stick.onpointerdown=e=>{stickPointer=e.pointerId;stick.setPointerCapture(e.pointerId);moveStick(e);};stick.onpointermove=e=>{if(e.pointerId===stickPointer)moveStick(e);};
stick.onpointerup=stick.onpointercancel=()=>{stickPointer=null;touch.forward=touch.right=0;stick.firstChild.style.transform='';};
$('touch-interact').onclick=interact;
for(const[name,key]of[['jump','jump'],['crouch','crouch'],['sprint','sprint']]){
  const b=$(`touch-${name}`);b.onpointerdown=e=>{touch[key]=true;b.setPointerCapture(e.pointerId);};b.onpointerup=b.onpointercancel=()=>touch[key]=false;
}
try{$('continue').hidden=!Core.parseSnapshot(localStorage.getItem(STORAGE));}catch{}
syncDoors();setCamera();world.update(state,0);requestAnimationFrame(frame);
if(testing)window.__nostalgia={
  get state(){return structuredClone(state);},get player(){return {x:player.x,y:player.y,z:player.z,yaw:player.yaw,pitch:player.pitch,crouched:player.crouched,grounded:player.grounded};},
  get audio(){return audio.debug;},get focus(){setCamera();return cast()?.id??null;},get enemy(){return enemy?{...enemy}:null;},
  step(seconds){for(let t=0;t<seconds;t+=1/60)tick(Math.min(1/60,seconds-t));renderer.render(scene,camera);},
  place(x,z,y=0){if(!player.free(x,z,y,player.height,world.colliders))throw new Error(`Blocked test placement ${x},${z}`);player.teleport(x,z,y);setCamera();},
  lookAt(x,y,z){const dx=x-player.x,dy=y-(player.y+player.eye),dz=z-player.z;player.yaw=Math.atan2(-dx,-dz);player.pitch=Math.atan2(dy,Math.hypot(dx,dz));setCamera();},
  target(id){const entry=interactions.get(id);if(!entry)throw Error(id);return entry.mesh.getWorldPosition(new THREE.Vector3()).toArray();},
  interact,save,load,get running(){return running;},get drawCalls(){return renderer.info.render.calls;}
};
