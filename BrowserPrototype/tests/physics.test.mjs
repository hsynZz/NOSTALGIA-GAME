import test from 'node:test';
import assert from 'node:assert/strict';
import {Player} from '../src/physics.js';
const simulate=(p,input,boxes=[],seconds=1)=>{for(let i=0;i<Math.round(seconds*90);i++)p.step(1/90,input,boxes);};
test('walking sprint crouch and normalized diagonal have actual speeds',()=>{
  for(const [input,speed]of [[{forward:1},3.2],[{forward:1,sprint:true},5.2],[{forward:1,crouch:true},1.5],[{forward:1,right:1},3.2]]){
    const p=new Player();p.teleport(0,0);p.yaw=0;simulate(p,input);assert.ok(Math.abs(Math.hypot(p.x,p.z)-speed)<.001);
  }
});
test('capsule cannot cross a wall at sprint speed and can slide',()=>{
  const p=new Player();p.teleport(0,0);p.yaw=0;const b=[{minX:-10,maxX:10,minZ:-2,maxZ:-1.9,height:3}];
  simulate(p,{forward:1,right:1,sprint:true},b);assert.ok(p.z>=-1.59);assert.ok(p.x>3);
});
test('jump lands and held space does not autojump',()=>{
  const p=new Player();simulate(p,{jump:true},[],.3);assert.ok(p.y>.5);simulate(p,{jump:true},[],1.5);assert.equal(p.y,0);
  p.step(1/90,{},[]);p.step(1/90,{jump:true},[]);assert.ok(p.y>0);
});
test('crouch fits under a lintel and standing waits for clearance',()=>{
  const p=new Player();p.teleport(0,0);const b=[{minX:-1,maxX:1,minZ:-1,maxZ:1,baseY:1.2,height:1.5}];
  p.step(.01,{crouch:true},b);p.step(.01,{},b);assert.equal(p.crouched,true);
  p.teleport(3,0);p.step(.01,{},b);assert.equal(p.crouched,false);
});
test('jump can land on furniture and a closed door blocks traversal',()=>{
  const p=new Player();p.teleport(0,0,1);p.vy=-1;
  const b={minX:-1,maxX:1,minZ:-1,maxZ:1,height:.7};simulate(p,{},[b],.6);assert.equal(p.y,.7);
  p.teleport(0,0);p.yaw=0;const door={minX:-1,maxX:1,minZ:-1.1,maxZ:-1,height:2.4,enabled:true};
  simulate(p,{forward:1},[door]);assert.ok(p.z>-.7);door.enabled=false;simulate(p,{forward:1},[door]);assert.ok(p.z<-3);
});
