// World and map helpers extracted from renderer.js.

const TELEPORT_COOLDOWN_MS = 500;
let nextTeleportAt = 0;
let worldSurfaceGrid = null;
let canopyZones = [];
let lastSurfaceType = "ground";
let nextRippleAt = 0;
let lastMoveTick = 0;
const MOVE_FRAME_MS = 16;

let lastAvatarFx = { surface: "", depth: "", brightness: "", canopy: null };
let currentVisualQuality = "high";

let activeRegionPois = [];

function getMapBounds() {
  const mapW = Math.max(1, refs.worldMap?.clientWidth || 0);
  const mapH = Math.max(1, refs.worldMap?.clientHeight || 0);
  const avatarW = Math.max(1, refs.playerAvatar?.clientWidth || 22);
  const avatarH = Math.max(1, refs.playerAvatar?.clientHeight || 30);
  return {
    maxX: Math.max(0, mapW - avatarW),
    maxY: Math.max(0, mapH - avatarH)
  };
}


function regionPoiVisitId(poiId, chapterId = currentChapterId()) {
  return String(chapterId) + "|" + String(state.world.regionX) + "," + String(state.world.regionY) + "|" + String(poiId);
}

function regionPoiDefs(chapterId = currentChapterId()) {
  return REGION_POI_LIBRARY[chapterId] || REGION_POI_LIBRARY.camp || [];
}

function poiDistanceToPlayer(poi) {
  const px = Number(state.world.x || 0) + 11;
  const py = Number(state.world.y || 0) + 15;
  return Math.hypot(Number(poi?.x || 0) - px, Number(poi?.y || 0) - py);
}

function nearestRegionPoi(includeVisited = false) {
  if (!Array.isArray(activeRegionPois) || !activeRegionPois.length) return null;
  const list = includeVisited
    ? activeRegionPois.slice()
    : activeRegionPois.filter((p) => !p.visited);
  if (!list.length) return null;
  list.sort((a, b) => poiDistanceToPlayer(a) - poiDistanceToPlayer(b));
  return list[0] || null;
}

function regionPoiSummary(chapterId = currentChapterId()) {
  const defs = regionPoiDefs(chapterId);
  if (!defs.length) return { total: 0, done: 0, nearest: null };
  let done = 0;
  for (const def of defs) {
    const id = regionPoiVisitId(def.id, chapterId);
    if ((state.storyRuntime?.poiVisited || []).includes(id)) done += 1;
  }
  return { total: defs.length, done, nearest: nearestRegionPoi(false) };
}

function regionPoiTag(poi) {
  if (!poi) return "";
  if (poi.visited) return "\u5df2\u63a2\u7d22";
  return "\u53ef\u4ea4\u4e92";
}

function buildRegionPoiMarkers(mapWidth, mapHeight, seed, rand, visualQuality) {
  ensureGameplayState();
  if (!refs.worldMap) return;

  const defs = regionPoiDefs();
  activeRegionPois = [];
  if (!defs.length) return;

  const count = Math.min(defs.length, visualQuality === "low" ? 2 : 3);
  const start = Math.abs(seed * 17 + state.world.day * 3) % defs.length;
  const marginX = 46;
  const marginY = 42;

  for (let i = 0; i < count; i += 1) {
    const def = defs[(start + i) % defs.length];
    const id = regionPoiVisitId(def.id);
    const visited = (state.storyRuntime?.poiVisited || []).includes(id);
    const x = marginX + Math.floor(rand() * Math.max(20, mapWidth - marginX * 2));
    const y = marginY + Math.floor(rand() * Math.max(20, mapHeight - marginY * 2));

    const marker = document.createElement("div");
    marker.className = "map-poi" + (visited ? " visited" : "");
    marker.dataset.poiId = def.id;
    marker.dataset.poiType = def.type || "watch";
    marker.dataset.tag = def.name || "POI";
    marker.style.left = x + "px";
    marker.style.top = y + "px";
    marker.title = (def.name || "POI") + " - " + regionPoiTag({ visited: visited });
    marker.addEventListener("click", (event) => {
      event.stopPropagation();
      interactRegionPoi(def.id);
    });
    refs.worldMap.appendChild(marker);

    activeRegionPois.push({
      id: def.id,
      x,
      y,
      visited,
      def
    });
  }
}

function interactRegionPoi(poiId, fromGuide = false) {
  ensureGameplayState();
  const poi = (activeRegionPois || []).find((p) => p.id === poiId);
  if (!poi) return showToast("\u5f53\u524d\u533a\u57df\u6682\u65e0\u53ef\u4ea4\u4e92\u5730\u6807");

  const visitId = regionPoiVisitId(poi.id);
  if ((state.storyRuntime.poiVisited || []).includes(visitId)) {
    showToast((poi.def?.name || "\u5730\u6807") + "\u5df2\u7ecf\u63a2\u7d22\u8fc7\u4e86");
    return;
  }

  state.storyRuntime.poiVisited.push(visitId);
  const reward = poi.def?.reward || {};
  applyDecisionReward(reward);

  if (poi.def?.metric) addMissionMetric(poi.def.metric, 1);
  progressSideQuests("explore", 1);
  if ((poi.def?.metric || "") === "artifact_found") progressSideQuests("artifact", 1);
  if ((poi.def?.metric || "") === "boss_hits") progressSideQuests("combat", 1);
  grantCompanionGrowth("\u533a\u57df\u63a2\u7d22", (poi.def?.metric || "") === "artifact_found" ? 14 : 10, 1, { toast: false });

  const title = poi.def?.name || "\u65e0\u540d\u5730\u6807";
  const desc = poi.def?.desc || "\u4f60\u5728\u8fd9\u5904\u5730\u6807\u53d1\u73b0\u4e86\u65b0\u7ebf\u7d22\u3002";
  const rewardText = formatRewardInline(reward);
  addJournal("\u5730\u6807\u63a2\u7d22: " + title + "\u5df2\u88ab\u8bb0\u5f55\u5230\u65c5\u884c\u624b\u518c\u3002");
  pushStoryCard("event", "\u5730\u6807\u63a2\u7d22: " + title, desc + (rewardText ? ("\n\u5956\u52b1: " + rewardText) : ""));
  showToast("\u5b8c\u6210\u5730\u6807\u63a2\u7d22: " + title);

  if (fromGuide) appendLog("\u5267\u60c5\u6307\u5f15\u5df2\u5b8c\u6210\u4e00\u9879\u5730\u6807\u63a2\u7d22\u3002");
  buildWorldTiles();
  renderAll();
}

function interactNearestRegionPoi() {
  const poi = nearestRegionPoi(false);
  if (!poi) return showToast("\u9644\u8fd1\u6ca1\u6709\u53ef\u4ea4\u4e92\u5730\u6807");
  if (poiDistanceToPlayer(poi) > 160) {
    return showToast("\u79bb\u5730\u6807\u592a\u8fdc\uff0c\u53ef\u5148\u53cc\u51fb\u5730\u56fe\u4f20\u9001\u8fc7\u53bb\u3002");
  }
  interactRegionPoi(poi.id, true);
}
function clampPlayerPosition() {
  const bounds = getMapBounds();
  state.world.x = Math.max(0, Math.min(bounds.maxX, state.world.x));
  state.world.y = Math.max(0, Math.min(bounds.maxY, state.world.y));
}
function getPlayerSurfaceType() {
  if (!refs.playerAvatar || !worldSurfaceGrid) return "ground";
  const tile = Math.max(1, worldSurfaceGrid.tile || 32);
  const avatarW = Math.max(1, refs.playerAvatar.clientWidth || 22);
  const avatarH = Math.max(1, refs.playerAvatar.clientHeight || 30);
  const px = Math.max(0, Math.floor((state.world.x + avatarW * 0.5) / tile));
  const py = Math.max(0, Math.floor((state.world.y + avatarH * 0.78) / tile));
  const seed = Number(worldSurfaceGrid.seed || 0);
  const noiseA = (py * 23 + px * 31 + seed * 7) % 100;
  if (noiseA < 12) return "water";
  if (noiseA > 84) return "high";
  return "ground";
}

function isAvatarUnderCanopy() {
  if (!refs.playerAvatar || !refs.worldMap) return false;

  if (!canopyZones.length) {
    const landmarks = refs.worldMap.querySelectorAll(".map-landmark-tree,.map-landmark-settlement");
    canopyZones = Array.from(landmarks).map((lm) => {
      const left = Number.parseFloat(lm.style.left || "0");
      const top = Number.parseFloat(lm.style.top || "0");
      const width = Number.parseFloat(lm.style.width || "0");
      const height = Number.parseFloat(lm.style.height || "0");
      return {
        x: left - 6,
        y: top + height * 0.2,
        w: width + 12,
        h: height * 0.9
      };
    });
  }

  const avatarW = Math.max(1, refs.playerAvatar.clientWidth || 22);
  const avatarH = Math.max(1, refs.playerAvatar.clientHeight || 30);
  const footX = state.world.x + avatarW * 0.5;
  const footY = state.world.y + avatarH * 0.82;

  for (const zone of canopyZones) {
    if (footX >= zone.x && footX <= zone.x + zone.w && footY >= zone.y && footY <= zone.y + zone.h) {
      return true;
    }
  }
  return false;
}

function spawnWaterRipple(x, y, scale = 1) {
  if (!refs.worldMap) return;
  const ripple = document.createElement("div");
  ripple.className = "map-water-ripple";
  ripple.style.left = x + "px";
  ripple.style.top = y + "px";
  ripple.style.setProperty("--ripple-scale", String(scale));
  refs.worldMap.appendChild(ripple);
  setTimeout(() => ripple.remove(), 860);
}

function refreshAvatarEnvironmentFx(moved = false) {
  if (!refs.playerAvatar || !refs.worldMap) return;
  const surface = getPlayerSurfaceType();
  if (lastAvatarFx.surface !== surface) {
    refs.playerAvatar.dataset.surface = surface;
    lastAvatarFx.surface = surface;
  }

  const bounds = getMapBounds();
  const maxY = Math.max(1, bounds.maxY || 1);
  const depth = Math.max(0, Math.min(1, state.world.y / maxY));
  const depthText = depth.toFixed(3);
  if (lastAvatarFx.depth !== depthText) {
    refs.playerAvatar.style.setProperty("--avatar-depth", depthText);
    lastAvatarFx.depth = depthText;
  }

  const phase = refs.worldMap.dataset.time || "day";
  const timeBrightness = phase === "night" ? 0.88 : phase === "dawn" ? 0.95 : 1.03;
  const brightnessText = String(timeBrightness);
  const brightnessChanged = lastAvatarFx.brightness !== brightnessText;
  if (brightnessChanged) {
    refs.playerAvatar.style.setProperty("--avatar-time-brightness", brightnessText);
    lastAvatarFx.brightness = brightnessText;
  }

  const canUseCanopyFx = currentVisualQuality === "high";
  const underCanopy = canUseCanopyFx ? isAvatarUnderCanopy() : false;
  const canopyChanged = lastAvatarFx.canopy !== underCanopy;
  if (canopyChanged) {
    refs.playerAvatar.classList.toggle("under-canopy", underCanopy);
    lastAvatarFx.canopy = underCanopy;
  }

  if (currentVisualQuality === "low") {
    if (refs.playerAvatar.style.filter) refs.playerAvatar.style.filter = "";
  } else if (canopyChanged || brightnessChanged) {
    const saturation = underCanopy ? 0.84 : 1;
    refs.playerAvatar.style.filter = "saturate(" + saturation + ") brightness(" + timeBrightness + ")";
  }

  if (moved && surface === "water" && refs.playerAvatar) {
    const now = Date.now();
    const rippleInterval = currentVisualQuality === "low" ? 220 : (currentVisualQuality === "medium" ? 160 : 110);
    if (now >= nextRippleAt) {
      const avatarW = Math.max(1, refs.playerAvatar.clientWidth || 22);
      const avatarH = Math.max(1, refs.playerAvatar.clientHeight || 30);
      const rippleScale = lastSurfaceType === "water" ? 1 : 1.28;
      spawnWaterRipple(state.world.x + avatarW * 0.5, state.world.y + avatarH * 0.86, rippleScale);
      nextRippleAt = now + rippleInterval;
    }
  }

  lastSurfaceType = surface;
}

function refreshWorldMapLayout(force = false) {
  if (!refs.worldMap || state.currentView !== "gameplay") return;
  const w = Math.floor(refs.worldMap.clientWidth || 0);
  const h = Math.floor(refs.worldMap.clientHeight || 0);
  if (w < 40 || h < 40) return;

  const prevW = Number(refs.worldMap.dataset.renderW || 0);
  const prevH = Number(refs.worldMap.dataset.renderH || 0);
  if (force || w !== prevW || h !== prevH) {
    buildWorldTiles();
    refs.worldMap.dataset.renderW = String(w);
    refs.worldMap.dataset.renderH = String(h);
  }
  updateHUD();
}

function getVisualQuality() {
  const fps = Number(state.settings?.fpsLimit || 60);
  const w = Math.max(1, refs.worldMap?.clientWidth || 0);
  const h = Math.max(1, refs.worldMap?.clientHeight || 0);
  const area = w * h;
  if (fps <= 30 || area > 1000000) return "low";
  if (fps <= 45 || area > 700000) return "medium";
  return "high";
}

function updateRegionName() {
  const names = ["\u6668\u98ce\u5e73\u539f","\u5e7d\u5f71\u6797\u5730","\u7070\u5ca9\u5ce1\u8c37","\u53e4\u4ee3\u9057\u8ff9","\u88c2\u9699\u8fb9\u5883"];
  const idx = Math.abs(state.world.regionX * 3 + state.world.regionY) % names.length;
  state.world.location = `${names[idx]} [${state.world.regionX},${state.world.regionY}]`;
}

function updateAvatarPlacement() {
  if (!refs.playerAvatar) return;
  if (refs.worldMap && !refs.worldMap.contains(refs.playerAvatar)) {
    refs.worldMap.appendChild(refs.playerAvatar);
  }
  clampPlayerPosition();
  const xText = `${state.world.x}px`;
  const yText = `${state.world.y}px`;
  if (refs.playerAvatar.style.left !== xText) refs.playerAvatar.style.left = xText;
  if (refs.playerAvatar.style.top !== yText) refs.playerAvatar.style.top = yText;
  if (refs.playerAvatar.dataset.profession !== state.player.profession) {
    refs.playerAvatar.dataset.profession = state.player.profession;
  }
}
function updatePositionHUD() {
  const nearestPoi = nearestRegionPoi(false);
  const poiText = nearestPoi
    ? (" | \u9644\u8fd1\u5730\u6807: " + (nearestPoi.def?.name || nearestPoi.id) + "(" + Math.round(poiDistanceToPlayer(nearestPoi)) + ")")
    : "";
  setTextIfChanged(refs.playerPos, `\u5750\u6807: (${state.world.x}, ${state.world.y})${poiText}`);
  const locationEl = document.getElementById("gameLocation");
  setTextIfChanged(locationEl, `\u5730\u70b9: ${state.world.location}`);
  renderNpcPanel();
}

function movePlayer(dx, dy) {
  if (state.currentView !== "gameplay" || !refs.worldMap || !refs.playerAvatar) return;

  const nowTick = Date.now();
  const moveFrameMs = currentVisualQuality === "low" ? 26 : (currentVisualQuality === "medium" ? 20 : MOVE_FRAME_MS);
  if (nowTick - lastMoveTick < moveFrameMs) return;
  lastMoveTick = nowTick;

  const bounds = getMapBounds();
  const maxX = bounds.maxX;
  const maxY = bounds.maxY;
  if (maxX < 16 || maxY < 16) {
    refreshWorldMapLayout(true);
    return;
  }

  let nx = state.world.x + dx;
  let ny = state.world.y + dy;
  let regionChanged = false;

  const entryX = Math.max(16, Math.floor(maxX * 0.16));
  const entryY = Math.max(22, Math.floor(maxY * 0.18));
  if (nx < 0) { state.world.regionX -= 1; nx = Math.max(0, maxX - entryX); regionChanged = true; }
  if (nx > maxX) { state.world.regionX += 1; nx = Math.min(maxX, entryX); regionChanged = true; }
  if (ny < 0) { state.world.regionY -= 1; ny = Math.max(0, maxY - entryY); regionChanged = true; }
  if (ny > maxY) { state.world.regionY += 1; ny = Math.min(maxY, entryY); regionChanged = true; }

  state.world.x = Math.max(0, Math.min(maxX, nx));
  state.world.y = Math.max(0, Math.min(maxY, ny));
  if (Math.abs(dx) > Math.abs(dy)) {
    refs.playerAvatar.dataset.facing = dx >= 0 ? "right" : "left";
  } else if (Math.abs(dy) > 0) {
    refs.playerAvatar.dataset.facing = dy >= 0 ? "down" : "up";
  }
  refs.playerAvatar.classList.add("walking");
  clearTimeout(movePlayer.walkTimer);
  movePlayer.walkTimer = setTimeout(() => refs.playerAvatar?.classList.remove("walking"), 140);
  if (regionChanged) {
    updateRegionName();
    buildWorldTiles();
    clampPlayerPosition();
    appendLog("\u8fdb\u5165\u65b0\u533a\u57df: [" + state.world.regionX + "," + state.world.regionY + "]");
    updateHUD();
  } else {
    updateAvatarPlacement();
    updatePositionHUD();
    refreshAvatarEnvironmentFx(true);
  }
}

function spawnTeleportMarker(x, y) {
  if (!refs.worldMap) return;
  refs.worldMap.querySelectorAll(".map-teleport-marker").forEach((n) => n.remove());

  const marker = document.createElement("div");
  marker.className = "map-teleport-marker";
  marker.style.left = x + "px";
  marker.style.top = y + "px";
  refs.worldMap.appendChild(marker);

  setTimeout(() => marker.remove(), 520);
}

function teleportPlayerTo(clientX, clientY) {
  if (state.currentView !== "gameplay" || !refs.worldMap || !refs.playerAvatar) return;

  const now = Date.now();
  if (now < nextTeleportAt) {
    const remain = Math.max(1, nextTeleportAt - now);
    showToast("\u4f20\u9001\u51b7\u5374\u4e2d: " + remain + "\u6beb\u79d2");
    return;
  }

  const rect = refs.worldMap.getBoundingClientRect();
  if (rect.width < 40 || rect.height < 40) {
    refreshWorldMapLayout(true);
    return;
  }

  const avatarW = Math.max(1, refs.playerAvatar.clientWidth || 22);
  const avatarH = Math.max(1, refs.playerAvatar.clientHeight || 30);
  const localX = clientX - rect.left - avatarW / 2;
  const localY = clientY - rect.top - avatarH / 2;
  const bounds = getMapBounds();

  const prevX = state.world.x;
  state.world.x = Math.max(0, Math.min(bounds.maxX, Math.round(localX)));
  state.world.y = Math.max(0, Math.min(bounds.maxY, Math.round(localY)));

  refs.playerAvatar.dataset.facing = state.world.x >= prevX ? "right" : "left";
  refs.playerAvatar.classList.add("walking");
  clearTimeout(teleportPlayerTo.walkTimer);
  teleportPlayerTo.walkTimer = setTimeout(() => refs.playerAvatar?.classList.remove("walking"), 140);

  updateAvatarPlacement();
  updatePositionHUD();
  refreshAvatarEnvironmentFx(true);
  spawnTeleportMarker(state.world.x + Math.round(avatarW / 2), state.world.y + Math.round(avatarH / 2));
  nextTeleportAt = Date.now() + TELEPORT_COOLDOWN_MS;
  appendLog("Teleport: (" + state.world.x + "," + state.world.y + ")");
  showToast("\u5df2\u4f20\u9001");
}

function buildWorldTiles() {
  if (!refs.worldMap) return;
  canopyZones = [];
  refs.worldMap.querySelectorAll(".world-row,.map-ornament,.map-river,.map-river-svg,.map-road-svg,.map-contour-svg,.map-lake,.map-edge,.map-path,.map-mist,.map-landmark,.map-cloud-shadow,.map-sun-shaft,.map-vignette-spot,.map-water-ripple,.map-npc,.map-poi").forEach((n) => n.remove());

  const biomes = [
    { name: "forest", ground: ["#315f42", "#2a553a", "#3d6f4d"], water: ["#2f5f76", "#244b62"], high: ["#536a45", "#607a52"], ornaments: ["#4f7f57", "#406c48"], landmarks: ["tree", "tree", "rock", "ruin", "settlement"] },
    { name: "desert", ground: ["#8a7244", "#7c653b", "#937c4c"], water: ["#466176", "#3c5365"], high: ["#9a8556", "#a18f61"], ornaments: ["#c3a268", "#aa8f59"], landmarks: ["rock", "ruin", "rock", "tree", "settlement"] },
    { name: "snow", ground: ["#8ca0af", "#7f93a1", "#99adba"], water: ["#4f6f88", "#426078"], high: ["#c6d4de", "#d2dfe7"], ornaments: ["#e2ebf1", "#cfdce5"], landmarks: ["ruin", "rock", "tree", "rock", "settlement"] },
    { name: "swamp", ground: ["#42553a", "#37492f", "#4b613f"], water: ["#30535b", "#2b4850"], high: ["#5e7a4a", "#4d6840"], ornaments: ["#73905d", "#5f7e4d"], landmarks: ["tree", "rock", "tree", "ruin", "settlement"] },
    { name: "volcanic", ground: ["#604039", "#53352f", "#6a4941"], water: ["#6b3e33", "#7e493b"], high: ["#8f584a", "#9d6454"], ornaments: ["#c06a57", "#a85b4b"], landmarks: ["rock", "ruin", "rock", "tree"] }
  ];

  const seed = Math.abs(state.world.regionX * 97 + state.world.regionY * 131);
  const biome = biomes[seed % biomes.length];
  refs.worldMap.dataset.biome = biome.name;

  const atmosphereByBiome = {
    forest: { haze: "rgba(158, 205, 154, 0.24)", cloud: "rgba(194, 222, 196, 0.18)", shaft: "rgba(245, 226, 182, 0.23)" },
    desert: { haze: "rgba(228, 194, 126, 0.21)", cloud: "rgba(232, 209, 169, 0.16)", shaft: "rgba(255, 223, 160, 0.24)" },
    snow: { haze: "rgba(216, 234, 246, 0.25)", cloud: "rgba(232, 242, 248, 0.2)", shaft: "rgba(214, 231, 249, 0.2)" },
    swamp: { haze: "rgba(136, 164, 128, 0.21)", cloud: "rgba(160, 188, 150, 0.16)", shaft: "rgba(218, 233, 175, 0.18)" },
    volcanic: { haze: "rgba(188, 125, 104, 0.2)", cloud: "rgba(169, 119, 108, 0.15)", shaft: "rgba(246, 182, 146, 0.2)" }
  };
  const atmosphere = atmosphereByBiome[biome.name] || atmosphereByBiome.forest;
  refs.worldMap.style.setProperty("--biome-haze", atmosphere.haze);
  refs.worldMap.style.setProperty("--biome-cloud", atmosphere.cloud);
  refs.worldMap.style.setProperty("--biome-shaft", atmosphere.shaft);

  const visualQuality = getVisualQuality();
  currentVisualQuality = visualQuality;
  refs.worldMap.dataset.quality = visualQuality;
  const tile = visualQuality === "low" ? 48 : (visualQuality === "medium" ? 40 : 32);
  const mapWidth = Math.max(320, refs.worldMap.clientWidth);
  const mapHeight = Math.max(240, refs.worldMap.clientHeight);
  const cols = Math.ceil(mapWidth / tile);
  const rows = Math.ceil(mapHeight / tile);
  const frag = document.createDocumentFragment();

  let randomState = seed + 17;
  const rand = () => {
    randomState = (randomState * 1103515245 + 12345) & 0x7fffffff;
    return randomState / 0x80000000;
  };

  for (let r = 0; r < rows; r += 1) {
    const row = document.createElement("div");
    row.className = "world-row";
    for (let c = 0; c < cols; c += 1) {
      const t = document.createElement("div");
      t.className = "world-tile";
      const noiseA = (r * 23 + c * 31 + seed * 7) % 100;
      const noiseB = (r * 11 + c * 17 + seed * 13) % 100;
      const noiseC = (r * 29 + c * 7 + seed * 19) % 100;
      const nx = (c + 0.5) / Math.max(1, cols);
      const ny = (r + 0.5) / Math.max(1, rows);
      const centerDist = Math.hypot(nx - 0.5, ny - 0.5);
      const centerBoost = Math.max(0, 1 - centerDist * 1.45);
      let baseColor = biome.ground[noiseB % biome.ground.length];

      if (noiseA < 12) {
        baseColor = biome.water[noiseB % biome.water.length];
        t.style.boxShadow = "inset 0 0 0 1px rgba(170,220,255,0.17)";
      } else if (noiseA > 84) {
        baseColor = biome.high[noiseB % biome.high.length];
        t.style.boxShadow = "inset 0 0 0 1px rgba(255,255,255,0.12)";
      } else {
        baseColor = biome.ground[noiseB % biome.ground.length];
        if (noiseB % 9 === 0) {
          t.style.boxShadow = "inset 0 0 0 1px rgba(255,255,255,0.06)";
        }
      }
      t.style.backgroundColor = baseColor;

      if (visualQuality !== "low") {
        const lightAlpha = (0.025 + centerBoost * 0.08 + noiseC * 0.001).toFixed(3);
        const darkAlpha = (0.07 + (1 - centerBoost) * 0.11 + (noiseA % 8) * 0.004).toFixed(3);
        t.style.backgroundImage = "linear-gradient(135deg, rgba(255,255,255," + lightAlpha + "), rgba(0,0,0," + darkAlpha + "))";
        if ((c + r + seed) % 5 === 0) {
          t.style.outline = "1px solid rgba(255,255,255,0.028)";
        }
      }
      row.appendChild(t);
    }
    frag.appendChild(row);
  }
  refs.worldMap.prepend(frag);

  const cloudCount = visualQuality === "low" ? 1 : (visualQuality === "medium" ? 2 : 3);
  for (let i = 0; i < cloudCount; i += 1) {
    const cloud = document.createElement("div");
    cloud.className = "map-cloud-shadow";
    const w = Math.floor(mapWidth * (0.24 + rand() * 0.25));
    const h = Math.floor(mapHeight * (0.12 + rand() * 0.14));
    cloud.style.width = w + "px";
    cloud.style.height = h + "px";
    cloud.style.left = Math.floor(rand() * Math.max(1, mapWidth - w)) + "px";
    cloud.style.top = Math.floor(rand() * Math.max(1, mapHeight - h)) + "px";
    cloud.style.opacity = (0.2 + rand() * 0.2).toFixed(2);
    cloud.style.animationDuration = (12 + rand() * 7).toFixed(2) + "s";
    cloud.style.animationDelay = (rand() * 3.2).toFixed(2) + "s";
    refs.worldMap.appendChild(cloud);
  }

  const shaftCount = visualQuality === "low" ? 1 : 2;
  for (let i = 0; i < shaftCount; i += 1) {
    const shaft = document.createElement("div");
    shaft.className = "map-sun-shaft";
    shaft.style.width = Math.floor(mapWidth * (0.2 + rand() * 0.22)) + "px";
    shaft.style.height = Math.floor(mapHeight * (0.95 + rand() * 0.24)) + "px";
    shaft.style.left = Math.floor(mapWidth * (0.06 + rand() * 0.7)) + "px";
    shaft.style.top = Math.floor(-mapHeight * (0.16 + rand() * 0.08)) + "px";
    shaft.style.opacity = (0.22 + rand() * 0.16).toFixed(2);
    shaft.style.transform = "rotate(" + (-22 + rand() * 44).toFixed(1) + "deg)";
    refs.worldMap.appendChild(shaft);
  }

  const vignetteCount = visualQuality === "low" ? 1 : (visualQuality === "medium" ? 2 : 3);
  for (let i = 0; i < vignetteCount; i += 1) {
    const spot = document.createElement("div");
    spot.className = "map-vignette-spot";
    const size = Math.floor(Math.min(mapWidth, mapHeight) * (0.34 + rand() * 0.2));
    spot.style.width = size + "px";
    spot.style.height = size + "px";
    spot.style.left = Math.floor(rand() * Math.max(1, mapWidth - size)) + "px";
    spot.style.top = Math.floor(rand() * Math.max(1, mapHeight - size)) + "px";
    spot.style.opacity = (0.15 + rand() * 0.12).toFixed(2);
    refs.worldMap.appendChild(spot);
  }

  const svgNS = "http://www.w3.org/2000/svg";
  const createRiverPath = (verticalFlow, bendScale = 1) => {
    const points = [];
    const steps = 6 + Math.floor(rand() * 3);
    if (verticalFlow) {
      const baseX = mapWidth * (0.2 + rand() * 0.6);
      const amp = (30 + rand() * 50) * bendScale;
      for (let i = 0; i <= steps; i += 1) {
        const t = i / steps;
        const y = -20 + t * (mapHeight + 40);
        const sway = Math.sin(t * Math.PI * (1.4 + rand() * 1.6) + rand() * 2.2) * amp;
        points.push({ x: baseX + sway, y });
      }
    } else {
      const baseY = mapHeight * (0.2 + rand() * 0.6);
      const amp = (28 + rand() * 44) * bendScale;
      for (let i = 0; i <= steps; i += 1) {
        const t = i / steps;
        const x = -20 + t * (mapWidth + 40);
        const sway = Math.sin(t * Math.PI * (1.2 + rand() * 1.8) + rand() * 2.0) * amp;
        points.push({ x, y: baseY + sway });
      }
    }

    let d = "M " + points[0].x.toFixed(1) + " " + points[0].y.toFixed(1);
    for (let i = 1; i < points.length; i += 1) {
      const prev = points[i - 1];
      const curr = points[i];
      const cx = ((prev.x + curr.x) / 2).toFixed(1);
      const cy = ((prev.y + curr.y) / 2).toFixed(1);
      d += " Q " + prev.x.toFixed(1) + " " + prev.y.toFixed(1) + " " + cx + " " + cy;
    }
    const last = points[points.length - 1];
    d += " T " + last.x.toFixed(1) + " " + last.y.toFixed(1);
    return d;
  };

  const appendRiverLayers = (svg, d, width, opacityScale = 1) => {
    const bank = document.createElementNS(svgNS, "path");
    bank.setAttribute("d", d);
    bank.setAttribute("class", "river-bank");
    bank.setAttribute("stroke-width", String(width + 12));
    bank.style.opacity = String(0.5 * opacityScale);

    const deep = document.createElementNS(svgNS, "path");
    deep.setAttribute("d", d);
    deep.setAttribute("class", "river-deep");
    deep.setAttribute("stroke-width", String(Math.max(4, Math.floor(width * 0.78))));
    deep.style.opacity = String(0.42 * opacityScale);

    const water = document.createElementNS(svgNS, "path");
    water.setAttribute("d", d);
    water.setAttribute("class", "river-water");
    water.setAttribute("stroke-width", String(width));
    water.style.opacity = String(0.94 * opacityScale);

    const foam = document.createElementNS(svgNS, "path");
    foam.setAttribute("d", d);
    foam.setAttribute("class", "river-foam");
    foam.setAttribute("stroke-width", String(Math.max(2, Math.floor(width * 0.2))));
    foam.style.opacity = String(0.7 * opacityScale);

    const glint = document.createElementNS(svgNS, "path");
    glint.setAttribute("d", d);
    glint.setAttribute("class", "river-glint");
    glint.setAttribute("stroke-width", String(Math.max(2, Math.floor(width * 0.22))));
    glint.style.opacity = String(0.84 * opacityScale);

    svg.appendChild(bank);
    svg.appendChild(deep);
    svg.appendChild(water);
    svg.appendChild(foam);
    svg.appendChild(glint);
  };

  const riverCount = visualQuality === "low" ? 1 : (visualQuality === "medium" ? (1 + Math.floor(rand() * 2)) : (2 + Math.floor(rand() * 2)));
  for (let i = 0; i < riverCount; i += 1) {
    const svg = document.createElementNS(svgNS, "svg");
    svg.setAttribute("viewBox", "0 0 " + mapWidth + " " + mapHeight);
    svg.setAttribute("width", String(mapWidth));
    svg.setAttribute("height", String(mapHeight));
    svg.classList.add("map-river-svg");

    const vertical = rand() > 0.45;
    const d = createRiverPath(vertical);
    const width = 18 + Math.floor(rand() * 16);
    appendRiverLayers(svg, d, width, 1);

    if (rand() > (visualQuality === "low" ? 0.95 : 0.52)) {
      const branchD = createRiverPath(!vertical, 0.56);
      appendRiverLayers(svg, branchD, Math.max(8, Math.floor(width * 0.6)), 0.68);
    }
    refs.worldMap.appendChild(svg);
  }

  const createRoutePath = (preferHorizontal) => {
    const points = [];
    const steps = 5 + Math.floor(rand() * 3);
    if (preferHorizontal) {
      const baseY = mapHeight * (0.18 + rand() * 0.64);
      const amp = 16 + rand() * 24;
      for (let i = 0; i <= steps; i += 1) {
        const t = i / steps;
        const x = -16 + t * (mapWidth + 32);
        const sway = Math.sin(t * Math.PI * (1.1 + rand() * 1.5) + rand() * 2.0) * amp;
        points.push({ x, y: baseY + sway });
      }
    } else {
      const baseX = mapWidth * (0.18 + rand() * 0.64);
      const amp = 14 + rand() * 22;
      for (let i = 0; i <= steps; i += 1) {
        const t = i / steps;
        const y = -16 + t * (mapHeight + 32);
        const sway = Math.sin(t * Math.PI * (1.0 + rand() * 1.4) + rand() * 2.2) * amp;
        points.push({ x: baseX + sway, y });
      }
    }
    let d = "M " + points[0].x.toFixed(1) + " " + points[0].y.toFixed(1);
    for (let i = 1; i < points.length; i += 1) {
      const prev = points[i - 1];
      const curr = points[i];
      const cx = ((prev.x + curr.x) / 2).toFixed(1);
      const cy = ((prev.y + curr.y) / 2).toFixed(1);
      d += " Q " + prev.x.toFixed(1) + " " + prev.y.toFixed(1) + " " + cx + " " + cy;
    }
    const last = points[points.length - 1];
    d += " T " + last.x.toFixed(1) + " " + last.y.toFixed(1);
    return d;
  };

  const roadCount = visualQuality === "low" ? 1 : (visualQuality === "medium" ? 2 : 3);
  for (let i = 0; i < roadCount; i += 1) {
    const roadSvg = document.createElementNS(svgNS, "svg");
    roadSvg.setAttribute("viewBox", "0 0 " + mapWidth + " " + mapHeight);
    roadSvg.setAttribute("width", String(mapWidth));
    roadSvg.setAttribute("height", String(mapHeight));
    roadSvg.classList.add("map-road-svg");

    const d = createRoutePath(rand() > 0.4);
    const roadWidth = 9 + Math.floor(rand() * 7);

    const base = document.createElementNS(svgNS, "path");
    base.setAttribute("d", d);
    base.setAttribute("class", "road-base");
    base.setAttribute("stroke-width", String(roadWidth + 4));

    const dust = document.createElementNS(svgNS, "path");
    dust.setAttribute("d", d);
    dust.setAttribute("class", "road-dust");
    dust.setAttribute("stroke-width", String(roadWidth + 10));

    const tread = document.createElementNS(svgNS, "path");
    tread.setAttribute("d", d);
    tread.setAttribute("class", "road-tread");
    tread.setAttribute("stroke-width", String(roadWidth));

    roadSvg.appendChild(dust);
    roadSvg.appendChild(base);
    roadSvg.appendChild(tread);
    refs.worldMap.appendChild(roadSvg);
  }

  const contourSvg = document.createElementNS(svgNS, "svg");
  contourSvg.setAttribute("viewBox", "0 0 " + mapWidth + " " + mapHeight);
  contourSvg.setAttribute("width", String(mapWidth));
  contourSvg.setAttribute("height", String(mapHeight));
  contourSvg.classList.add("map-contour-svg");
  const contourCount = visualQuality === "low" ? 2 : (visualQuality === "medium" ? 3 : 5);
  for (let i = 0; i < contourCount; i += 1) {
    const p = document.createElementNS(svgNS, "path");
    const y0 = mapHeight * (0.13 + i * 0.16 + rand() * 0.05);
    const d = "M -18 " + y0.toFixed(1)
      + " Q " + (mapWidth * 0.25).toFixed(1) + " " + (y0 + (rand() * 26 - 13)).toFixed(1)
      + " " + (mapWidth * 0.5).toFixed(1) + " " + (y0 + (rand() * 30 - 15)).toFixed(1)
      + " T " + (mapWidth + 20).toFixed(1) + " " + (y0 + (rand() * 26 - 13)).toFixed(1);
    p.setAttribute("d", d);
    p.setAttribute("class", "contour-line");
    p.setAttribute("stroke-width", String(1 + (i % 2)));
    contourSvg.appendChild(p);
  }
  refs.worldMap.appendChild(contourSvg);

  const lakeCount = visualQuality === "low" ? 1 : (visualQuality === "medium" ? 2 : 3);
  for (let i = 0; i < lakeCount; i += 1) {
    const lake = document.createElement("div");
    lake.className = "map-lake";
    const w = 56 + Math.floor(rand() * 110);
    const h = 30 + Math.floor(rand() * 58);
    lake.style.width = w + "px";
    lake.style.height = h + "px";
    lake.style.left = Math.floor(rand() * Math.max(1, mapWidth - w - 6)) + "px";
    lake.style.top = Math.floor(rand() * Math.max(1, mapHeight - h - 6)) + "px";
    lake.style.transform = "rotate(" + (-16 + rand() * 32).toFixed(1) + "deg)";
    lake.style.opacity = (0.55 + rand() * 0.27).toFixed(2);
    lake.style.filter = "saturate(" + (0.92 + rand() * 0.35).toFixed(2) + ") brightness(" + (0.86 + rand() * 0.28).toFixed(2) + ")";
    refs.worldMap.appendChild(lake);
  }

  ["top", "right", "bottom", "left"].forEach((edge) => {
    const e = document.createElement("div");
    e.className = "map-edge map-edge-" + edge;
    refs.worldMap.appendChild(e);
  });

  const mistCount = visualQuality === "low" ? 1 : (visualQuality === "medium" ? 2 : 3);
  for (let i = 0; i < mistCount; i += 1) {
    const mist = document.createElement("div");
    mist.className = "map-mist";
    const mw = Math.floor(mapWidth * (0.35 + rand() * 0.28));
    const mh = Math.floor(mapHeight * (0.28 + rand() * 0.22));
    mist.style.width = mw + "px";
    mist.style.height = mh + "px";
    mist.style.left = Math.floor(rand() * Math.max(1, mapWidth - mw)) + "px";
    mist.style.top = Math.floor(rand() * Math.max(1, mapHeight - mh)) + "px";
    mist.style.animationDelay = (rand() * 2.5).toFixed(2) + "s";
    refs.worldMap.appendChild(mist);
  }

  const ornamentCount = visualQuality === "low" ? 10 : (visualQuality === "medium" ? 18 : 28);
  for (let i = 0; i < ornamentCount; i += 1) {
    const o = document.createElement("div");
    o.className = "map-ornament";
    const size = 8 + ((seed + i * 11) % 28);
    const ox = (seed * 13 + i * 73 + Math.floor(rand() * 37)) % Math.max(1, mapWidth - size - 8);
    const oy = (seed * 17 + i * 59 + Math.floor(rand() * 41)) % Math.max(1, mapHeight - size - 8);
    o.style.left = ox + "px";
    o.style.top = oy + "px";
    o.style.width = size + "px";
    o.style.height = size + "px";
    o.style.borderRadius = (24 + rand() * 36).toFixed(1) + "%";
    o.style.opacity = (0.52 + rand() * 0.35).toFixed(2);
    o.style.transform = "rotate(" + (-18 + rand() * 36).toFixed(1) + "deg)";
    o.style.background = biome.ornaments[(i + seed) % biome.ornaments.length];
    refs.worldMap.appendChild(o);
  }

  const landmarkCount = visualQuality === "low" ? 10 : (visualQuality === "medium" ? 14 : 20);
  for (let i = 0; i < landmarkCount; i += 1) {
    const lm = document.createElement("div");
    const kind = biome.landmarks[Math.floor(rand() * biome.landmarks.length)] || "rock";
    lm.className = "map-landmark map-landmark-" + kind;
    const size = kind === "settlement" ? (22 + Math.floor(rand() * 16)) : (14 + Math.floor(rand() * 24));
    lm.style.width = size + "px";
    lm.style.height = size + "px";
    lm.style.left = Math.floor(rand() * Math.max(1, mapWidth - size - 4)) + "px";
    lm.style.top = Math.floor(rand() * Math.max(1, mapHeight - size - 4)) + "px";
    lm.style.opacity = (0.72 + rand() * 0.25).toFixed(2);
    lm.style.transform = "rotate(" + (-10 + rand() * 20).toFixed(1) + "deg)";
    refs.worldMap.appendChild(lm);
  }

  buildRegionPoiMarkers(mapWidth, mapHeight, seed, rand, visualQuality);
  worldSurfaceGrid = { tile, mapWidth, mapHeight, seed };
  lastSurfaceType = getPlayerSurfaceType();

  clampPlayerPosition();
  refreshNpcPopulation(true);
  renderNpcPanel();
  const biomeLabelMap = { forest: "\u68ee\u6797", desert: "\u8352\u6f20", snow: "\u96ea\u539f", swamp: "\u6cbc\u6cfd", volcanic: "\u706b\u5c71" };
  appendLog("\u5730\u5f62: " + (biomeLabelMap[biome.name] || biome.name) + " [" + state.world.regionX + "," + state.world.regionY + "]");
  refreshAvatarEnvironmentFx(false);
}

window.FantasyWorldMapModule = Object.freeze({
  getMapBounds,
  regionPoiVisitId,
  regionPoiDefs,
  poiDistanceToPlayer,
  nearestRegionPoi,
  regionPoiSummary,
  regionPoiTag,
  buildRegionPoiMarkers,
  interactRegionPoi,
  interactNearestRegionPoi,
  clampPlayerPosition,
  getPlayerSurfaceType,
  isAvatarUnderCanopy,
  spawnWaterRipple,
  refreshAvatarEnvironmentFx,
  refreshWorldMapLayout,
  getVisualQuality,
  updateRegionName,
  updateAvatarPlacement,
  updatePositionHUD,
  movePlayer,
  spawnTeleportMarker,
  teleportPlayerTo,
  buildWorldTiles
});
