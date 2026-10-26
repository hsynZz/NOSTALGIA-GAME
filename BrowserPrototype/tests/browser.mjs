import assert from 'node:assert/strict';
import {chromium} from 'playwright';
import {createServer} from 'node:http';
import {readFile,writeFile,mkdir} from 'node:fs/promises';
import {resolve} from 'node:path';
const output=resolve('test-results');await mkdir(output,{recursive:true});
const html=await readFile('dist/index.html','utf8');
const server=createServer((req,res)=>{if(req.url.startsWith('/favicon')){res.writeHead(204);res.end();return;}res.writeHead(200,{'Content-Type':'text/html; charset=utf-8'});res.end(html);});
await new Promise(r=>server.listen(0,'127.0.0.1',r));
const url=`http://127.0.0.1:${server.address().port}/?test=1`;
const browser=await chromium.launch({executablePath:process.env.CHROMIUM_PATH||'/usr/bin/chromium',headless:true,args:['--no-sandbox','--use-angle=swiftshader','--enable-unsafe-swiftshader']});
const context=await browser.newContext({viewport:{width:1280,height:720}}),page=await context.newPage();
const errors=[],checks=[],requests=[];page.on('pageerror',e=>errors.push(e.message));page.on('request',r=>requests.push(r.url()));
const api=fn=>page.evaluate(fn);
const step=seconds=>page.evaluate(s=>window.__nostalgia.step(s),seconds);
const place=(x,z,y=0)=>page.evaluate(p=>window.__nostalgia.place(...p),[x,z,y]);
const look=(x,y,z)=>page.evaluate(p=>window.__nostalgia.lookAt(...p),[x,y,z]);
const state=()=>api(()=>window.__nostalgia.state);
const player=()=>api(()=>window.__nostalgia.player);
const note=text=>{checks.push(text);console.log(`PASS ${text}`);};
async function action(expected){assert.equal(await api(()=>window.__nostalgia.focus),expected,`actual raycast should focus ${expected}`);await page.keyboard.press('e');}
async function move(seconds,key='w'){await page.keyboard.down(key);await step(seconds);await page.keyboard.up(key);}
try{
  await page.goto(url);await page.waitForFunction(()=>!!window.__nostalgia);
  await page.waitForFunction(()=>window.__nostalgia.drawCalls>0);await page.screenshot({path:resolve(output,'01-menu.png')});
  await page.click('#start');await page.waitForFunction(()=>window.__nostalgia.running);
  assert.equal(await api(()=>document.getElementById('game').getContext('webgl2').getParameter(WebGL2RenderingContext.VERSION)).then(v=>v.includes('WebGL 2')),true);
  assert.equal((await state()).stage,0);
  const mouseBefore=await player();await page.mouse.move(600,300);await page.mouse.down();await page.mouse.move(660,320,{steps:3});await page.mouse.up();assert.notEqual((await player()).yaw,mouseBefore.yaw);
  await page.keyboard.press('Escape');assert.equal(await page.locator('#menu').isVisible(),true);const pausedMinute=(await state()).minute;await step(.5);assert.equal((await state()).minute,pausedMinute);await page.click('#start');await page.waitForFunction(()=>window.__nostalgia.running);
  note('Actual WebGL2 startup, mouse look, pause/resume and user-gesture audio activation');
  await move(.7);assert.ok(Math.hypot((await player()).x+12,(await player()).z)>1.5);
  await page.keyboard.down('ControlLeft');await step(.1);assert.equal((await player()).crouched,true);await page.keyboard.up('ControlLeft');await step(.1);
  await page.keyboard.down('Space');await step(.25);assert.ok((await player()).y>.4);await page.keyboard.up('Space');await step(1);
  await place(-10,0);await look(-2,1.58,0);await page.keyboard.down('ShiftLeft');await move(.3);await page.keyboard.up('ShiftLeft');assert.ok((await player()).x>-8.5);note('Actual Shift sprint input increases movement speed');
  await place(-10,1.3);await look(-8.5,.985,2.6);await action('photo');assert.equal((await state()).stage,1);assert.equal(await page.locator('#photo').isVisible(),true);
  await page.keyboard.press('j');await page.screenshot({path:resolve(output,'02-bedroom.png')});
  await page.keyboard.press('F5');const previousSave=await api(()=>localStorage.getItem('nostalgia.browser.01'));assert.ok(previousSave);await place(-8.5,2.8,.92);await page.keyboard.press('F5');assert.equal(await api(()=>localStorage.getItem('nostalgia.browser.01')),previousSave);note('Keyboard walking, crouch, jump, photo and safe rejection of saves on furniture');
  await place(-3.5,0);await look(-2,1.2,0);await move(1);assert.ok((await player()).x<-2.3);await action('door-bedroom');await step(1);
  await move(1);assert.ok((await player()).x>-2);
  await place(-1,0);await look(-1,1.2,-1.1);assert.equal(await api(()=>window.__nostalgia.focus),'door-bedroom');
  await api(()=>{window.__nostalgia.interact();window.__nostalgia.place(-2,0);});await step(1);await page.keyboard.press('F5');assert.equal(await api(()=>localStorage.getItem('nostalgia.browser.01')),previousSave);await look(-5,1.58,0);await move(.6);assert.ok((await player()).x<-3);note('Closed door collision, opening, keyboard traversal and safe recovery while a door closes');
  await place(4.5,0);await look(6,1.2,0);await action('door-front');await step(1);await move(1);assert.ok((await player()).x>6);note('Second house door and front-yard access');
  await place(55,0);await look(60,1.5,0);await move(1);
  assert.equal((await state()).loops,1);assert.equal((await state()).stage,1);assert.ok((await player()).x<11);note('Early street loop returns to the original neighborhood without quest bypass');
  await step(3.1);await place(12,-9.5);await look(12,1.2,-11.28);await action('house-entry');assert.ok((await player()).z>27);
  await step(3.1);await place(-18.2,32);await look(-19.5,1.2,32);await action('house-return');assert.ok((await player()).z<0);note('Oversized neighbor interior entry and oriented return portal');
  await place(19,8);await look(24,1.1,12.5);await step(1.3);
  assert.equal((await state()).history[0].count,1);
  await page.waitForFunction(()=>window.__nostalgia.audio.rms>.0001,{timeout:10000});
  const audioEvidence=await api(()=>window.__nostalgia.audio);assert.equal(audioEvidence.state,'running');note('Localized garden event actually starts WebAudio and produces nonzero analyser output');
  await place(20,5.3);await look(24,1.3,11);await page.screenshot({path:resolve(output,'03-garden.png')});
  await place(22,12.5);await step(.1);assert.equal((await api(()=>window.__nostalgia.audio)).gardenFading,true);
  await place(19,8);await step(2);assert.equal((await state()).history[0].count,1);note('Laughter fades within three meters and does not repeat');
  await place(18.9,9);await look(20.3,1.35,9);await action('compare');assert.equal((await state()).stage,2);
  await place(24,9.5);await look(24,.9,11);await action('birthday');assert.equal((await state()).stage,3);assert.ok((await state()).minute>=1204);note('Photo discrepancy and birthday restore cause the night/route change');
  await place(28,0);await look(35,2.3,8.5);await step(2.5);assert.equal((await state()).history[1].count,1);assert.ok(await api(()=>!!window.__nostalgia.enemy));
  await page.screenshot({path:resolve(output,'04-homecomer.png')});await look(0,1.6,0);await step(3);assert.ok((await api(()=>window.__nostalgia.enemy)).travel>.5);assert.ok((await api(()=>window.__nostalgia.enemy)).travel<=1.5);await step(11.1);assert.equal(await api(()=>window.__nostalgia.enemy),null);note('Visible Homecomer starts once, approaches while looking away and retires within its bounded duration');
  const quiet=45-((await state()).elapsed-(await state()).lastShock)+.2;if(quiet>0)await step(quiet);
  await place(-17,32);await step(4.3);assert.equal((await state()).history[2].count,1);note('Night window knock respects the shared forty-five second peace interval');
  await place(55,0);await look(60,1.5,0);await move(1);
  assert.equal((await state()).stage,4);assert.ok((await player()).x>=72);note('Restored birthday unlocks the changed street destination');
  await step(3.2);await place(85.8,4.5);await look(87.4,1.2,4.5);await action('playhouse-entry');assert.ok((await player()).z<7);note('Playhouse remains locked until the dark-lamp clue');
  await place(83.5,3.3);await look(85,1.05,3.3);await action('lamp');assert.equal((await state()).stage,5);
  await page.keyboard.press('F5');const saved=await state();assert.ok(await api(()=>!!localStorage.getItem('nostalgia.browser.01')));note('Dark lamp clue and persistent browser save');
  await step(3.2);await place(85.8,4.5);await look(87.4,1.2,4.5);await action('playhouse-entry');assert.ok((await player()).z>28);
  await look(90,1.5,30);await move(5.1);assert.ok((await player()).x>87);await look(90,1.2,30);await action('exit');
  assert.equal((await state()).stage,6);assert.equal(await page.locator('#complete').isVisible(),true);
  const endMinute=(await state()).minute;await step(2);assert.equal((await state()).minute,endMinute);note('Keyboard traversal of impossible playhouse corridor and real completion screen');
  await page.screenshot({path:resolve(output,'05-complete.png')});
  await page.click('#end-load');await page.waitForFunction(()=>window.__nostalgia.running);
  assert.equal((await state()).stage,saved.stage);assert.equal((await state()).history[1].count,1);note('Load restores progression and consumed event history');
  await page.reload();await page.waitForFunction(()=>!!window.__nostalgia);assert.equal(await page.locator('#continue').isVisible(),true);await page.click('#continue');await page.waitForFunction(()=>window.__nostalgia.running);assert.equal((await state()).stage,5);note('Browser reload retains a usable save');
  // Exact delivery bytes, with networking disabled: no server/CDN assets are required.
  const offlineContext=await browser.newContext({offline:true,viewport:{width:960,height:540}}),offline=await offlineContext.newPage();
  const offlineErrors=[];offline.on('pageerror',e=>offlineErrors.push(e.message));
  await offline.goto('about:blank?test=1');await offline.setContent(html,{waitUntil:'load'});await offline.waitForTimeout(500);await offline.click('#start');
  assert.equal(await offline.locator('#hud').isVisible(),true);assert.equal(offlineErrors.length,0);await offlineContext.close();note('Standalone delivery HTML executes and starts with browser networking disabled');
  // Touch emulation proves UI input wiring; this is not an iPhone/Safari hardware result.
  const touchContext=await browser.newContext({viewport:{width:844,height:390},hasTouch:true,isMobile:true}),phone=await touchContext.newPage();
  const touchErrors=[];phone.on('pageerror',e=>touchErrors.push(e.message));await phone.goto(url);await phone.waitForFunction(()=>!!window.__nostalgia);await phone.click('#start');
  assert.equal(await phone.locator('#touch').isVisible(),true);
  await phone.dispatchEvent('#touch-crouch','pointerdown',{pointerId:1});await phone.evaluate(()=>window.__nostalgia.step(.1));assert.equal(await phone.evaluate(()=>window.__nostalgia.player.crouched),true);
  await phone.dispatchEvent('#touch-crouch','pointerup',{pointerId:1});
  const cdp=await touchContext.newCDPSession(phone),bounds=await phone.locator('#stick').boundingBox();
  const touchPoint={x:bounds.x+bounds.width/2,y:bounds.y+bounds.height/2-30,id:1};
  const beforeTouch=await phone.evaluate(()=>window.__nostalgia.player);
  await cdp.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[touchPoint]});await phone.evaluate(()=>window.__nostalgia.step(.3));await cdp.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});
  assert.ok(Math.hypot((await phone.evaluate(()=>window.__nostalgia.player.x))-beforeTouch.x,(await phone.evaluate(()=>window.__nostalgia.player.z))-beforeTouch.z)>.4);
  const yawBefore=await phone.evaluate(()=>window.__nostalgia.player.yaw);
  await cdp.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:600,y:140,id:2}]});await cdp.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{x:650,y:145,id:2}]});await cdp.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});
  assert.notEqual(await phone.evaluate(()=>window.__nostalgia.player.yaw),yawBefore);
  await phone.evaluate(()=>{window.__nostalgia.place(-10,1.3);window.__nostalgia.lookAt(-8.5,.985,2.6);});await phone.click('#touch-interact');assert.equal(await phone.locator('#photo').isVisible(),true);await phone.click('#close-photo');await phone.click('#memory');assert.equal(await phone.locator('#photo').isVisible(),true);await phone.click('#close-photo');
  await phone.screenshot({path:resolve(output,'06-touch.png')});assert.equal(touchErrors.length,0);await touchContext.close();note('Touch joystick, drag look, crouch, interaction and photo buttons work in mobile Chromium emulation');
  assert.deepEqual(errors,[]);
  assert.ok(requests.every(r=>r.startsWith(`http://127.0.0.1:${server.address().port}/`)));
  await writeFile(resolve(output,'browser-results.json'),JSON.stringify({result:'passed',completed_utc:new Date().toISOString(),checks,
    browser:await browser.version(),rendering:'actual Chromium WebGL2, SwiftShader software GPU',audio:audioEvidence,
    page_errors:errors,external_asset_requests:0,shortcuts:'Test-only free-space positioning between longer sections; keyboard movement, raycast E interactions, portals and quest transitions use the running game.',
    file_navigation:'Cloud Chromium blocks file:// by administrator policy. Standalone bytes tested via offline about:blank execution; Windows file-double-click remains untested.',
    iphone_hardware:'not tested',unreal:'not built or executed'},null,2)+'\n');
}catch(error){await page.screenshot({path:resolve(output,'failure.png')}).catch(()=>{});await writeFile(resolve(output,'failure.json'),JSON.stringify({error:String(error),checks,state:await state().catch(()=>null),player:await player().catch(()=>null),focus:await api(()=>window.__nostalgia.focus).catch(()=>null),pageErrors:errors},null,2));throw error;}
finally{await browser.close();await new Promise(r=>server.close(r));}
