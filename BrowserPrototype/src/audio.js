// Original, synthesized test sounds. The laughter is a voiced "ha-ha" approximation,
// not a recording of children. No files, services or third-party samples are loaded.
export class AudioDirector {
  constructor() {
    this.context = null;
    this.master = null;
    this.analyser = null;
    this.muted = false;
    this.garden = null;
    this.windowKnock = null;
    this.gardenStarts = 0;
    this.knockStarts = 0;
    this.buffers = new Map();
    this.samples = null;
    this.lastError = null;
  }

  async unlock() {
    try {
      if (!this.context) {
        const Context = globalThis.AudioContext || globalThis.webkitAudioContext;
        if (!Context) return false;
        this.context = new Context();
        this.master = this.context.createGain();
        this.master.gain.value = this.muted ? 0 : 0.72;
        this.analyser = this.context.createAnalyser();
        this.analyser.fftSize = 1024;
        this.samples = new Float32Array(this.analyser.fftSize);
        this.master.connect(this.analyser);
        this.analyser.connect(this.context.destination);
      }
      if (this.context.state !== 'running') await this.context.resume();
      return this.context.state === 'running';
    } catch (error) {
      this.lastError = String(error);
      return false;
    }
  }

  setMuted(value) {
    this.muted = !!value;
    if (this.master) this.master.gain.setTargetAtTime(this.muted ? 0 : 0.72, this.context.currentTime, 0.025);
  }

  buffer(kind) {
    if (this.buffers.has(kind)) return this.buffers.get(kind);
    const rate = this.context.sampleRate;
    const seconds = kind === 'garden' ? 6 : 1.8;
    const buffer = this.context.createBuffer(1, Math.ceil(rate * seconds), rate);
    const data = buffer.getChannelData(0);
    let seed = 2004;
    const random = () => { seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0; return seed / 2147483648 - 1; };
    if (kind === 'garden') {
      const syllables = [0.14, 0.42, 0.71, 1.4, 1.65, 1.92, 2.7, 2.95, 3.23, 4.18, 4.45, 4.74, 5.08];
      for (let n = 0; n < syllables.length; n++) {
        const start = syllables[n], duration = n % 3 === 2 ? 0.27 : 0.22;
        const pitch = 340 + (n % 3) * 36 + Math.sin(n * 2.7) * 30;
        let phase = 0;
        for (let i = Math.floor(start * rate); i < Math.min(data.length, (start + duration) * rate); i++) {
          const t = i / rate - start, u = t / duration;
          // Breathy consonant onset followed by a short rising/falling voiced vowel.
          const envelope = Math.pow(Math.sin(Math.PI * u), 1.2) * Math.min(1, t / 0.015);
          const frequency = pitch * (1.18 - 0.37 * u + 0.025 * Math.sin(t * 44));
          phase += 2 * Math.PI * frequency / rate;
          let voice = 0;
          for (let h = 1; h <= 9; h++) {
            const hz = frequency * h;
            const formant = 0.13 / h + 0.55 * Math.exp(-(((hz - 1050) / 330) ** 2))
              + 0.3 * Math.exp(-(((hz - 1950) / 450) ** 2));
            voice += Math.sin(phase * h) * formant;
          }
          const breath = random() * 0.16 * Math.exp(-t * 35);
          data[i] += (voice * 0.24 + breath) * envelope;
        }
      }
    } else {
      for (const start of [0.12, 0.47, 0.91]) {
        for (let i = Math.floor(start * rate); i < Math.min(data.length, (start + 0.23) * rate); i++) {
          const t = i / rate - start;
          data[i] += (random() * 0.22 * Math.exp(-t * 70)
            + Math.sin(2 * Math.PI * 280 * t) * 0.4 * Math.exp(-t * 35)
            + Math.sin(2 * Math.PI * 670 * t) * 0.16 * Math.exp(-t * 50));
        }
      }
    }
    this.buffers.set(kind, buffer);
    return buffer;
  }

  start(kind, origin, volume) {
    if (!this.context || this.context.state !== 'running' || this.muted) return null;
    try {
      const source = this.context.createBufferSource();
      source.buffer = this.buffer(kind);
      const gain = this.context.createGain();
      gain.gain.value = volume;
      const panner = this.context.createPanner();
      panner.panningModel = 'equalpower';
      panner.distanceModel = 'linear';
      panner.refDistance = 1;
      panner.maxDistance = 11;
      panner.rolloffFactor = 1;
      if (panner.positionX) {
        panner.positionX.value = origin[0]; panner.positionY.value = origin[1]; panner.positionZ.value = origin[2];
      } else panner.setPosition(...origin);
      source.connect(gain); gain.connect(panner); panner.connect(this.master);
      const active = { source, gain, panner, fading: false, kind };
      source.onended = () => {
        source.disconnect(); gain.disconnect(); panner.disconnect();
        if (this.garden === active) this.garden = null;
        if (this.windowKnock === active) this.windowKnock = null;
      };
      source.start();
      return active;
    } catch (error) {
      this.lastError = String(error);
      return null;
    }
  }

  playGarden() {
    if (this.garden) return false;
    this.garden = this.start('garden', [24, 1.1, 12.5], 0.9);
    if (this.garden) this.gardenStarts++;
    return !!this.garden;
  }

  knock() {
    if (this.windowKnock) return false;
    this.windowKnock = this.start('knock', [-9, 1.5, 36.5], 0.65);
    if (this.windowKnock) this.knockStarts++;
    return !!this.windowKnock;
  }

  fade(active, duration = 0.35) {
    if (!active || active.fading) return;
    active.fading = true;
    const now = this.context.currentTime;
    active.gain.gain.cancelScheduledValues(now);
    active.gain.gain.setValueAtTime(active.gain.gain.value, now);
    active.gain.gain.linearRampToValueAtTime(0, now + duration);
    try { active.source.stop(now + duration + 0.01); } catch { /* Already ended. */ }
  }

  update(player, district) {
    if (!this.context) return;
    const listener = this.context.listener;
    const x = player.x, y = (player.y || 0) + 1.5, z = player.z;
    const fx = -Math.sin(player.yaw || 0), fz = -Math.cos(player.yaw || 0);
    if (listener.positionX) {
      listener.positionX.value = x; listener.positionY.value = y; listener.positionZ.value = z;
      listener.forwardX.value = fx; listener.forwardY.value = 0; listener.forwardZ.value = fz;
      listener.upX.value = 0; listener.upY.value = 1; listener.upZ.value = 0;
    } else {
      listener.setPosition(x, y, z); listener.setOrientation(fx, 0, fz, 0, 1, 0);
    }
    if (this.garden && (district !== 2 || Math.hypot(x - 24, z - 12.5) < 3)) this.fade(this.garden);
    if (this.windowKnock && district !== 3) this.fade(this.windowKnock, 0.2);
  }

  stop() {
    for (const active of [this.garden, this.windowKnock]) {
      if (!active) continue;
      try { active.source.stop(); } catch { /* Already ended. */ }
      active.source.disconnect(); active.gain.disconnect(); active.panner.disconnect();
    }
    this.garden = null;
    this.windowKnock = null;
  }

  get debug() {
    let rms = 0, peak = 0;
    if (this.analyser && this.samples) {
      this.analyser.getFloatTimeDomainData(this.samples);
      for (const v of this.samples) { rms += v * v; peak = Math.max(peak, Math.abs(v)); }
      rms = Math.sqrt(rms / this.samples.length);
    }
    return { state: this.context?.state ?? 'locked', muted: this.muted, gardenPlaying: !!this.garden,
      knockPlaying: !!this.windowKnock, gardenFading: !!this.garden?.fading, gardenStarts: this.gardenStarts,
      knockStarts: this.knockStarts, activeSources: Number(!!this.garden) + Number(!!this.windowKnock), rms, peak,
      sampleRate: this.context?.sampleRate ?? null, lastError: this.lastError };
  }
}
