// NPC and story guide helpers extracted from renderer.js.

function getStoryGuideChecklist() {
  ensureGameplayState();
  const c = chapter();
  const status = getStoryProgressStatus(c);
  const arcNodes = CHAPTER_ARCS[c.id] || [];
  const sideRequired = Math.max(1, state.story.chapterIndex + 1);
  const sideDone = Number(state.storyRuntime.completedSideQuests || 0);
  const relation = npcChapterRelationshipSummary(c.id);
  const poi = regionPoiSummary(c.id);
  const support = chapterSupportSummary(c.id);
  const rumors = typeof rumorSummary === "function"
    ? rumorSummary(c.id)
    : { total: 0, completed: 0, remaining: 0, current: null, currentStatus: null };

  const items = [
    { done: Boolean(state.story.choice), text: "\u786e\u8ba4\u5267\u60c5\u8def\u7ebf\uff08\u79e9\u5e8f / \u5747\u8861 / \u6df7\u6c8c\uff09" },
    { done: status.objectiveRemaining === 0, text: "\u5b8c\u6210\u4e3b\u76ee\u6807\uff1a" + status.mainKey + " " + status.progress + "/" + status.target }
  ];

  if (arcNodes.length) {
    const doneArc = Math.max(0, arcNodes.length - status.arcRemaining);
    items.push({ done: status.arcRemaining === 0, text: "\u5b8c\u6210\u4e3b\u7ebf\u8282\u70b9\uff1a" + doneArc + "/" + arcNodes.length });
  }

  items.push({
    done: !status.pendingDecision,
    text: status.pendingDecision ? ("\u5904\u7406\u5267\u60c5\u6295\u62e9\uff1a" + status.pendingDecision.title) : "\u672c\u7ae0\u5267\u60c5\u6295\u62e9\u5df2\u5904\u7406"
  });

  items.push({ done: status.sideNeed === 0, text: "\u5b8c\u6210\u652f\u7ebf\u4efb\u52a1\uff1a" + sideDone + "/" + sideRequired });

  if (status.mission.total) {
    items.push({
      done: status.mission.remaining === 0,
      text: "\u5b8c\u6210\u7ae0\u8282\u4efb\u52a1\uff1a" + status.mission.done + "/" + status.mission.total + (status.mission.next ? ("\uff08\u4e0b\u4e00\u9879\uff1a" + status.mission.next.title + "\uff09") : "")
    });
  }

  if (relation.total) {
    items.push({
      done: relation.unlocked >= relation.total,
      text: "\u89e3\u9501\u5173\u7cfb\u4e8b\u4ef6\uff1a" + relation.unlocked + "/" + relation.total
    });
  }

  if (poi.total) {
    items.push({
      done: poi.done >= poi.total,
      text: "\u63a2\u7d22\u533a\u57df\u5730\u6807\uff1a" + poi.done + "/" + poi.total + (poi.nearest ? ("\uff08\u9644\u8fd1\uff1a" + (poi.nearest.def?.name || poi.nearest.id) + "\uff09") : "")
    });
  }

  if (rumors.total) {
    const rumorBlocker = typeof rumorBlockerText === "function"
      ? rumorBlockerText(rumors.current, rumors.currentStatus)
      : (rumors.currentStatus?.blockers?.[0] || "\u7ee7\u7eed\u63a8\u8fdb\u7ae0\u8282\u7ebf\u7d22");
    items.push({
      done: rumors.remaining === 0,
      text: rumors.current
        ? ("\u63a8\u8fdb\u7ae0\u8282\u4f20\u95fb\uff1a" + rumors.completed + "/" + rumors.total + "\uff08\u5f53\u524d\uff1a" + rumors.current.title + (rumors.currentStatus?.ok ? "\uff0c\u53ef\u76f4\u63a5\u89e6\u53d1" : "\uff0c\u4ecd\u9700\uff1a" + rumorBlocker) + "\uff09")
        : ("\u7ae0\u8282\u4f20\u95fb\u5df2\u5168\u90e8\u5b8c\u6210\uff1a" + rumors.completed + "/" + rumors.total)
    });
  }

  if (support.total) {
    const supportBlocker = typeof supportPrimaryBlockerText === "function"
      ? supportPrimaryBlockerText(support.current, support.currentStatus, c.id)
      : (support.currentStatus?.blockers?.[0] || "\u7ee7\u7eed\u51c6\u5907\u534f\u4f5c\u6761\u4ef6");
    items.push({
      done: support.remaining === 0,
      text: support.current
        ? ("\u63a8\u8fdb\u7ae0\u8282\u534f\u4f5c\uff1a" + support.completed + "/" + support.total + "\uff08\u5f53\u524d\uff1a" + support.current.title + (support.currentStatus?.ok ? "\uff0c\u6761\u4ef6\u5df2\u6ee1\u8db3" : "\uff0c\u4ecd\u9700\uff1a" + supportBlocker) + "\uff09")
        : ("\u7ae0\u8282\u534f\u4f5c\u5df2\u5168\u90e8\u5b8c\u6210\uff1a" + support.completed + "/" + support.total)
    });
  }

  items.push({
    done: status.canAdvance,
    text: status.canAdvance ? "\u53ef\u63a8\u8fdb\u7ae0\u8282\uff08\u70b9\u51fb\u63a8\u8fdb\u7ae0\u8282\uff09" : "\u7ae0\u8282\u63a8\u8fdb\u6761\u4ef6\u5c1a\u672a\u6ee1\u8db3"
  });

  return { c, status, items, relation, poi, support, rumors };
}

function getStoryGuidePrimaryAction(pack) {
  const status = pack.status;
  if (!state.story.choice) return { id: "btnCommitChoice", label: "\u786e\u8ba4\u5267\u60c5\u8def\u7ebf" };
  if (status.pendingDecision) return { id: "btnStoryDecision", label: "\u5904\u7406\u5267\u60c5\u6295\u62e9" };

  if (status.mainKey === "\u9996\u9886" && status.objectiveRemaining > 0) {
    if (!state.combat.active) return { id: "btnStartBoss", label: "\u5f00\u59cb Boss \u6218" };
    return { id: "btnStrikeBoss", label: "\u653b\u51fb Boss" };
  }

  if (status.sideNeed > 0) {
    return { id: "btnOpenNpcDialog", label: "\u4e0e NPC \u4ea4\u6d41\u9886\u53d6\u59d4\u6258" };
  }

  if (pack.poi?.total && pack.poi.done < pack.poi.total) {
    return { id: "actionInteractPoi", label: "\u63a2\u7d22\u533a\u57df\u5730\u6807" };
  }

  if (pack.rumors?.current && pack.rumors.currentStatus?.ok && typeof openCurrentRumorNpc === "function") {
    return { id: "actionOpenRumorNpc", label: "\u63a8\u8fdb\u7ae0\u8282\u4f20\u95fb\uff1a" + pack.rumors.current.title };
  }

  if (pack.rumors?.current) {
    return { id: "btnOpenNpcDialog", label: "\u51c6\u5907\u7ae0\u8282\u4f20\u95fb\u6761\u4ef6" };
  }

  if (pack.relation?.total && pack.relation.unlocked < pack.relation.total) {
    return { id: "btnOpenNpcDialog", label: "\u63a8\u8fdb NPC \u5173\u7cfb\u4e8b\u4ef6" };
  }

  if (status.mission.remaining > 0 && status.mission.next) {
    const m = status.mission.next;
    if (m.metric === "boss_hits" || m.metric === "boss_kills") {
      if (!state.combat.active) return { id: "btnStartBoss", label: "\u5f00\u59cb Boss \u6218" };
      return { id: "btnStrikeBoss", label: m.metric === "boss_kills" ? "\u51fb\u8d25 Boss" : "\u653b\u51fb Boss" };
    }
    return { id: m.actionId || "btnExplore", label: m.actionLabel || "\u7ee7\u7eed\u63a2\u7d22\u63a8\u8fdb" };
  }

  if (pack.support?.current && pack.support.currentStatus?.ok) {
    return { id: "btnBuildProject", label: pack.support.current.actionLabel || ("\u63a8\u8fdb\u7ae0\u8282\u534f\u4f5c\uff1a" + pack.support.current.title) };
  }

  if (status.arcRemaining > 0 || status.objectiveRemaining > 0) {
    return { id: "btnExplore", label: "\u7ee7\u7eed\u63a2\u7d22\u63a8\u8fdb" };
  }

  if (status.canAdvance) return { id: "btnAdvanceChapter", label: "\u63a8\u8fdb\u7ae0\u8282" };
  return null;
}

function seededNpcRandom(seed) {
  let value = (seed >>> 0) || 1;
  return () => {
    value = (value * 1664525 + 1013904223) >>> 0;
    return value / 4294967296;
  };
}

function ensureNpcData() {
  ensureGameplayState();
  if (!state.npcs.spawnKey) state.npcs.spawnKey = "";
  if (!state.npcs.selectedId) state.npcs.selectedId = "";
  if (!state.npcs.chainFlags) state.npcs.chainFlags = {};
  if (!Array.isArray(state.npcs.chapterRewards)) state.npcs.chapterRewards = [];
  if (!Array.isArray(state.npcs.relationshipEvents)) state.npcs.relationshipEvents = [];
}

function npcDistanceToPlayer(npc) {
  const px = state.world.x + 11;
  const py = state.world.y + 15;
  return Math.hypot((npc.x || 0) - px, (npc.y || 0) - py);
}

function npcQuestId(npc) {
  return "npcq_" + currentChapterId() + "_" + String(npc?.key || "unknown");
}

function npcChainId(npc) {
  return String(npc?.chapterId || currentChapterId()) + "::" + String(npc?.key || "unknown");
}

function npcChainFlags(npc) {
  ensureNpcData();
  const id = npcChainId(npc);
  if (!state.npcs.chainFlags[id]) {
    state.npcs.chainFlags[id] = { talk: false, quest: false, bond: false, rewarded: false };
  }
  return state.npcs.chainFlags[id];
}

function npcChainProgress(npc) {
  const chapterId = String(npc?.chapterId || currentChapterId());
  const flags = npcChainFlags(npc);
  const lines = (NPC_STORY_CHAINS[chapterId]?.[npc?.key] || [
    "\u89e3\u9501\u60c5\u62a5\u7ebf\u7d22",
    "\u5b8c\u6210\u59d4\u6258\u4ea4\u4ed8",
    "\u5efa\u7acb\u9635\u8425\u9ed8\u5951"
  ]).slice(0, 3);
  while (lines.length < 3) lines.push("\u5267\u60c5\u8282\u70b9");
  const doneCount = Number(flags.talk) + Number(flags.quest) + Number(flags.bond);
  return { lines, doneCount, completed: doneCount >= 3, flags };
}

function npcChapterChainSummary(chapterId = currentChapterId()) {
  const pool = NPC_PROFILES[chapterId] || [];
  if (!pool.length) return { completed: 0, total: 0 };
  let completed = 0;
  for (const base of pool) {
    const info = npcChainProgress({ chapterId: chapterId, key: base.key });
    if (info.completed) completed += 1;
  }
  return { completed, total: pool.length };
}

function npcTrustValue(npcId) {
  ensureNpcData();
  const v = Number(state.npcs.trust[npcId] || 0);
  return Math.max(0, Math.min(10, v));
}

function npcTrustLabel(v) {
  if (v >= 9) return "\u751f\u6b7b\u4e4b\u4ea4";
  if (v >= 7) return "\u9ad8\u5ea6\u4fe1\u8d56";
  if (v >= 5) return "\u53ef\u9760\u6218\u53cb";
  if (v >= 3) return "\u521d\u6b65\u8ba4\u53ef";
  return "\u964c\u751f\u5173\u7cfb";
}

function npcRelationshipEventId(chapterId, npcKey, eventId) {
  return String(chapterId) + "::" + String(npcKey) + "::rel::" + String(eventId);
}

function npcRelationshipDefsFor(npc) {
  return NPC_RELATIONSHIP_EVENTS[npc?.key] || [];
}

function npcRelationshipEventStatus(npc) {
  ensureNpcData();
  const chapterId = String(npc?.chapterId || currentChapterId());
  const defs = npcRelationshipDefsFor(npc);
  if (!defs.length) return { unlocked: 0, total: 0, nextTrust: null };
  let unlocked = 0;
  let nextTrust = null;
  for (const def of defs) {
    const id = npcRelationshipEventId(chapterId, npc?.key || "unknown", def.id);
    if ((state.npcs.relationshipEvents || []).includes(id)) {
      unlocked += 1;
      continue;
    }
    if (nextTrust === null || Number(def.trust || 0) < nextTrust) nextTrust = Number(def.trust || 0);
  }
  return { unlocked, total: defs.length, nextTrust };
}

function npcChapterRelationshipSummary(chapterId = currentChapterId()) {
  ensureNpcData();
  const pool = NPC_PROFILES[chapterId] || [];
  if (!pool.length) return { unlocked: 0, total: 0 };
  let total = 0;
  let unlocked = 0;
  for (const base of pool) {
    const defs = NPC_RELATIONSHIP_EVENTS[base.key] || [];
    total += defs.length;
    for (const def of defs) {
      const id = npcRelationshipEventId(chapterId, base.key, def.id);
      if ((state.npcs.relationshipEvents || []).includes(id)) unlocked += 1;
    }
  }
  return { unlocked, total };
}

function applyNpcRelationshipReward(reward) {
  if (!reward) return;
  applyDecisionReward(reward);
  const rep = reward.rep || {};
  if (typeof rep.wardens === "number") applyReputation("wardens", rep.wardens);
  if (typeof rep.arcanum === "number") applyReputation("arcanum", rep.arcanum);
  if (typeof rep.freeguild === "number") applyReputation("freeguild", rep.freeguild);
}

function npcTryTriggerRelationshipEvents(npc, reason = "") {
  ensureNpcData();
  if (!npc) return 0;
  const defs = npcRelationshipDefsFor(npc);
  if (!defs.length) return 0;
  const chapterId = String(npc.chapterId || currentChapterId());
  const trust = npcTrustValue(npc.id);
  let triggered = 0;

  for (const def of defs) {
    const id = npcRelationshipEventId(chapterId, npc.key, def.id);
    if ((state.npcs.relationshipEvents || []).includes(id)) continue;
    if (trust < Number(def.trust || 0)) continue;

    state.npcs.relationshipEvents.push(id);
    triggered += 1;
    applyNpcRelationshipReward(def.reward || {});
    addMissionMetric("relationship_events", 1, chapterId);

    const rewardText = formatRewardInline(def.reward || {});
    addJournal("NPC\u5173\u7cfb\u4e8b\u4ef6: " + npc.name + " - " + def.title);
    pushStoryCard(
      "event",
      "NPC\u5173\u7cfb\u4e8b\u4ef6: " + npc.name,
      def.title + " | " + (def.body || "")
        + (rewardText ? ("\n\u5956\u52b1: " + rewardText) : "")
        + (reason ? ("\n\u6765\u6e90: " + reason) : "")
    );
    showToast("\u89e3\u9501\u5173\u7cfb\u4e8b\u4ef6: " + npc.name + " - " + def.title);
  }

  return triggered;
}

function npcTradePreview(npc) {
  const trust = npcTrustValue(npc.id);
  const profile = NPC_TRADE_PROFILES[npc.tradeType] || NPC_TRADE_PROFILES.supply;
  const cost = Math.max(profile.costFloor, profile.costBase - trust * 2);
  const bonus = trust >= 7 ? 2 : 1;
  let effect = "";
  if (profile.mode === "shard") {
    effect = "\u9057\u7269\u788e\u7247 +" + bonus;
  } else if (profile.mode === "weapon") {
    effect = "\u653b\u51fb +" + bonus;
  } else if (profile.mode === "armor") {
    effect = "\u9632\u5fa1 +" + bonus;
  } else {
    effect = "\u836f\u6c34 +" + bonus + "\uff0c\u751f\u547d\u56de\u590d";
  }
  return { trust, profile, cost, bonus, effect };
}

function setNpcTrust(npcId, delta) {
  ensureNpcData();
  state.npcs.trust[npcId] = Math.max(0, Math.min(10, npcTrustValue(npcId) + delta));
}

function findNpcById(npcId) {
  ensureNpcData();
  return (state.npcs.activeMap || []).find((x) => x.id === npcId) || null;
}

function npcTryUnlockChapterReward(chapterId = currentChapterId()) {
  ensureNpcData();
  const rewardId = "chain_reward_" + chapterId;
  if ((state.npcs.chapterRewards || []).includes(rewardId)) return false;

  const summary = npcChapterChainSummary(chapterId);
  if (!summary.total || summary.completed < summary.total) return false;

  state.npcs.chapterRewards.push(rewardId);
  if (chapterId === "camp") {
    gain(60, 120);
    state.story.progress.explore += 1;
    applyReputation("wardens", 2);
  } else if (chapterId === "forest") {
    gain(80, 90);
    state.content.artifactShards += 2;
    state.story.progress.artifact += 1;
    applyReputation("arcanum", 2);
  } else {
    gain(100, 120);
    state.player.attackBonus += 1;
    state.player.defenseBonus += 1;
    state.story.progress.boss += 1;
    applyReputation("freeguild", 2);
  }

  addJournal("\u7ae0\u8282\u534f\u529b\u5b8c\u6210\uff1a" + chapter().name + " \u7684 NPC \u5267\u60c5\u7ebf\u5168\u90e8\u8fbe\u6210\u3002");
  pushStoryCard("chapter", "NPC\u534f\u529b\u5b8c\u6210", chapter().name + " \u83b7\u5f97\u9636\u6bb5\u6027\u589e\u63f4\u5956\u52b1\u3002");
  showToast("\u89e3\u9501\u7ae0\u8282 NPC \u534f\u529b\u5956\u52b1");
  return true;
}

function npcAdvanceChainStep(npc, step) {
  const flags = npcChainFlags(npc);
  if (!Object.prototype.hasOwnProperty.call(flags, step)) return false;
  if (flags[step]) return false;
  if (step === "bond" && npcTrustValue(npc.id) < 5) return false;

  flags[step] = true;
  const progress = npcChainProgress(npc);
  const stepIdx = Math.max(0, progress.doneCount - 1);
  const stepText = progress.lines[stepIdx] || "\u5267\u60c5\u63a8\u8fdb";
  addJournal("NPC\u5267\u60c5\u63a8\u8fdb: " + npc.name + " - " + stepText);
  pushStoryCard("arc",      "NPC\u5173\u7cfb\u4e8b\u4ef6: " + npc.name, stepText);

  if (progress.completed && !flags.rewarded) {
    flags.rewarded = true;
    applyReputation(npc.faction || "wardens", 1);
    gain(22, 35);
    showToast("\u5b8c\u6210 " + npc.name + " \u7684 NPC \u5267\u60c5\u7ebf");
    npcTryUnlockChapterReward(npc.chapterId || currentChapterId());
  }
  return true;
}

function refreshNpcPopulation(force = false) {
  ensureNpcData();
  if (!refs.worldMap) return;

  const mapWidth = Math.max(320, refs.worldMap.clientWidth || 320);
  const mapHeight = Math.max(240, refs.worldMap.clientHeight || 240);
  const chapterId = currentChapterId();
  const seed = Math.abs(state.world.regionX * 173 + state.world.regionY * 197 + state.story.chapterIndex * 71);
  const spawnKey = chapterId + "|" + state.world.regionX + "|" + state.world.regionY + "|" + mapWidth + "|" + mapHeight;
  if (!force && state.npcs.spawnKey === spawnKey && state.npcs.activeMap.length) return;

  const pool = NPC_PROFILES[chapterId] || NPC_PROFILES.camp;
  const rng = seededNpcRandom(seed + 19);
  const marginX = 36;
  const marginY = 34;

  state.npcs.activeMap = pool.map((base, idx) => {
    const spreadX = marginX + Math.floor(rng() * Math.max(20, mapWidth - marginX * 2));
    const spreadY = marginY + Math.floor(rng() * Math.max(20, mapHeight - marginY * 2));
    const id = chapterId + "_" + state.world.regionX + "_" + state.world.regionY + "_" + base.key + "_" + idx;
    return {
      ...base,
      id,
      x: spreadX,
      y: spreadY,
      chapterId
    };
  });
  state.npcs.spawnKey = spawnKey;

  if (!findNpcById(state.npcs.selectedId || "")) {
    state.npcs.selectedId = state.npcs.activeMap[0]?.id || "";
  }

  renderNpcMarkers();
}

function renderNpcMarkers() {
  if (!refs.worldMap) return;
  refs.worldMap.querySelectorAll(".map-npc").forEach((n) => n.remove());
  (state.npcs.activeMap || []).forEach((npc) => {
    const marker = document.createElement("div");
    marker.className = "map-npc";
    if (npc.id === state.npcs.selectedId) marker.classList.add("active");
    marker.dataset.faction = npc.faction || "wardens";
    marker.dataset.tag = npc.name || "NPC";
    marker.style.left = Math.round(npc.x) + "px";
    marker.style.top = Math.round(npc.y) + "px";
    marker.title = (npc.name || "NPC") + " - " + (npc.title || "");
    marker.addEventListener("click", (event) => {
      event.stopPropagation();
      openNpcDialogById(npc.id);
    });
    refs.worldMap.appendChild(marker);
  });
}

function npcQuestStatus(npc) {
  const id = npcQuestId(npc);
  const quest = (state.sideQuests || []).find((q) => q.id === id);
  if (!quest) return { state: "none", questId: id, quest: null };
  if (quest.completed) {
    const claimed = (state.npcs.claimedQuestRewards || []).includes(id);
    return { state: claimed ? "done" : "turnin", questId: id, quest };
  }
  return { state: "active", questId: id, quest };
}

function addNpcQuest(npc) {
  ensureNpcData();
  const tpl = NPC_QUEST_TEMPLATES[npc.questKind] || NPC_QUEST_TEMPLATES.explore;
  const id = npcQuestId(npc);
  const existing = (state.sideQuests || []).find((q) => q.id === id);
  if (existing) return existing;
  const trust = npcTrustValue(npc.id);
  const quest = {
    id,
    kind: npc.questKind,
    title: tpl.title + " - " + (npc.name || "NPC"),
    target: tpl.target + (trust >= 5 ? 1 : 0),
    progress: 0,
    completed: false,
    rewardGold: tpl.rewardGold + trust * 6,
    rewardExp: tpl.rewardExp + trust * 5,
    faction: npc.faction || "wardens"
  };
  state.sideQuests.push(quest);
  return quest;
}
function openNpcDialogById(npcId) {
  refreshNpcPopulation(false);
  const npc = findNpcById(npcId) || state.npcs.activeMap[0] || null;
  if (!npc) return showToast("\u9644\u8fd1\u6ca1\u6709\u53ef\u4ea4\u4e92 NPC");
  activeNpcDialogId = npc.id;
  state.npcs.selectedId = npc.id;
  npcTryTriggerRelationshipEvents(npc, "dialog");
  renderNpcDialog();
  openModal("modalNpcDialog");
}

function openNearestNpcDialog() {
  refreshNpcPopulation(false);
  const list = (state.npcs.activeMap || []).slice().sort((a, b) => npcDistanceToPlayer(a) - npcDistanceToPlayer(b));
  if (!list.length) return showToast("\u9644\u8fd1\u6682\u65e0 NPC");
  openNpcDialogById(list[0].id);
}

function renderNpcDialog() {
  const npc = findNpcById(activeNpcDialogId || state.npcs.selectedId || "");
  if (!npc) return;
  const trust = npcTrustValue(npc.id);
  const trustLabel = npcTrustLabel(trust);
  const qs = npcQuestStatus(npc);
  const dist = Math.round(npcDistanceToPlayer(npc));
  const chain = npcChainProgress(npc);
  const trade = npcTradePreview(npc);
  const chapterSummary = npcChapterChainSummary(npc.chapterId || currentChapterId());
  const relationship = npcRelationshipEventStatus(npc);
  const chapterRelation = npcChapterRelationshipSummary(npc.chapterId || currentChapterId());

  if (refs.npcDialogTitle) refs.npcDialogTitle.textContent = npc.name + " - " + npc.title;
  if (refs.npcDialogBody) {
    refs.npcDialogBody.textContent =
      "\u9635\u8425: " + (npc.faction || "-") + "\n" +
      "\u597d\u611f: " + trust + "/10 (" + trustLabel + ")\n" +
      "\u8ddd\u79bb: " + dist + "\n\n" +
      npc.dialogue + "\n\n" +
      "\u4e2a\u4eba\u5267\u60c5\u7ebf: " + chain.doneCount + "/3\n" +
      (chain.flags.talk ? "[\u2713] " : "[ ] ") + chain.lines[0] + "\n" +
      (chain.flags.quest ? "[\u2713] " : "[ ] ") + chain.lines[1] + "\n" +
      (chain.flags.bond ? "[\u2713] " : "[ ] ") + chain.lines[2] + "\n" +
      "\u5173\u7cfb\u4e8b\u4ef6: " + relationship.unlocked + "/" + relationship.total + (relationship.nextTrust === null ? " | \u5df2\u5168\u90e8\u89e3\u9501" : (" | \u4e0b\u4e00\u9608\u503c \u597d\u611f>=" + relationship.nextTrust)) + "\n\n" +
      (qs.state === "none" ? "\u53ef\u9886\u53d6\u65b0\u59d4\u6258\u3002" : qs.state === "active"
        ? ("\u59d4\u6258\u8fdb\u5ea6: " + Math.min(qs.quest.progress, qs.quest.target) + "/" + qs.quest.target)
        : (qs.state === "turnin" ? "\u59d4\u6258\u5df2\u5b8c\u6210\uff0c\u53ef\u63d0\u4ea4\u7ed3\u7b97\u3002" : "\u59d4\u6258\u5df2\u7ed3\u7b97\uff0c\u53ef\u4ee5\u7ee7\u7eed\u4ea4\u6d41\u3002"));
  }

  if (refs.npcActionSummary) {
    refs.npcActionSummary.textContent =
      "\u4ea4\u6613\u9884\u89c8: " + trade.profile.label + " | \u82b1\u8d39 " + trade.cost + " \u91d1\u5e01 -> " + trade.effect + "\n" +
      "\u4ea4\u8c08: \u63d0\u5347\u5173\u7cfb\u5e76\u89e3\u9501\u5267\u60c5\u7ebf\u7d22\n" +
      "\u59d4\u6258: \u9886\u53d6/\u63d0\u4ea4 NPC \u4e13\u5c5e\u4efb\u52a1\n" +
      "\u8bad\u7ec3\u6216\u8d60\u793c: \u5f53\u597d\u611f\u22655 \u53ef\u63a8\u8fdb\u7b2c\u4e09\u9636\u6bb5\u5267\u60c5\n" +
      "\u5173\u7cfb\u4e8b\u4ef6: " + relationship.unlocked + "/" + relationship.total + "\n" +
      "\u7ae0\u8282\u534f\u529b\u8fdb\u5ea6: " + chapterSummary.completed + "/" + chapterSummary.total + " | \u7ae0\u8282\u5173\u7cfb\u89e3\u9501 " + chapterRelation.unlocked + "/" + chapterRelation.total;
  }
}

function npcActionTalk() {
  const npc = findNpcById(activeNpcDialogId || "");
  if (!npc) return;
  const trustGain = npcTrustValue(npc.id) >= 8 ? 0 : 1;
  if (trustGain > 0) setNpcTrust(npc.id, trustGain);
  progressSideQuests("explore", 1);
  addMissionMetric("npc_contacts", 1);
  npcAdvanceChainStep(npc, "talk");
  npcTryTriggerRelationshipEvents(npc, "talk");
  addJournal("\u4e0e " + npc.name + " \u4ea4\u8c08\uff1a" + npc.dialogue);
  pushStoryCard("event", "NPC: " + npc.name, npc.dialogue);
  showToast("\u4f60\u4e0e " + npc.name + " \u4ea4\u6362\u4e86\u7ebf\u7d22");
  renderAll();
  renderNpcDialog();
}

function npcActionQuest() {
  const npc = findNpcById(activeNpcDialogId || "");
  if (!npc) return;
  const status = npcQuestStatus(npc);
  if (status.state === "none") {
    const q = addNpcQuest(npc);
    addMissionMetric("npc_contacts", 1);
    setNpcTrust(npc.id, 1);
    npcTryTriggerRelationshipEvents(npc, "quest_accept");
    addJournal("\u63a5\u53d6\u59d4\u6258\uff1a" + q.title);
    showToast("\u5df2\u9886\u53d6 " + npc.name + " \u7684\u59d4\u6258");
    renderAll();
    renderNpcDialog();
    return;
  }

  if (status.state === "active") {
    showToast("\u59d4\u6258\u8fd8\u5728\u8fdb\u884c\u4e2d");
    renderNpcDialog();
    return;
  }

  if (status.state === "turnin") {
    state.npcs.claimedQuestRewards.push(status.questId);
    addMissionMetric("npc_quests_completed", 1);
    gain(26, 55);
    applyReputation(npc.faction || "wardens", 1);
    setNpcTrust(npc.id, 2);
    npcAdvanceChainStep(npc, "quest");
    npcTryTriggerRelationshipEvents(npc, "quest_turnin");
    addJournal("\u5b8c\u6210\u4ea4\u4ed8\uff1a" + status.quest.title + " (" + npc.name + ")");
    showToast("\u5df2\u63d0\u4ea4\u59d4\u6258\u5956\u52b1");
    renderAll();
    renderNpcDialog();
    return;
  }

  showToast("\u6b64 NPC \u59d4\u6258\u5df2\u7ed3\u7b97\u3002");
}

function npcActionTrade() {
  const npc = findNpcById(activeNpcDialogId || "");
  if (!npc) return;
  const trade = npcTradePreview(npc);
  if (state.player.gold < trade.cost) return showToast("\u91d1\u5e01\u4e0d\u8db3\uff0c\u9700\u8981 " + trade.cost);

  state.player.gold -= trade.cost;
  if (trade.profile.mode === "shard") {
    state.content.artifactShards += trade.bonus;
    progressSideQuests("artifact", 1);
    showToast("\u4ea4\u6613\u6210\u529f\uff1a\u788e\u7247 +" + trade.bonus);
  } else if (trade.profile.mode === "weapon") {
    state.player.attackBonus += trade.bonus;
    progressSideQuests("combat", 1);
    showToast("\u4ea4\u6613\u6210\u529f\uff1a\u653b\u51fb +" + trade.bonus);
  } else if (trade.profile.mode === "armor") {
    state.player.defenseBonus += trade.bonus;
    state.player.hp = Math.min(state.player.maxHp, state.player.hp + 4 * trade.bonus);
    progressSideQuests("explore", 1);
    showToast("\u4ea4\u6613\u6210\u529f\uff1a\u9632\u5fa1 +" + trade.bonus);
  } else {
    state.player.potions += trade.bonus;
    state.player.hp = Math.min(state.player.maxHp, state.player.hp + 12 * trade.bonus);
    progressSideQuests("explore", 1);
    showToast("\u4ea4\u6613\u6210\u529f\uff1a\u836f\u6c34 +" + trade.bonus);
  }

  setNpcTrust(npc.id, 1);
  npcTryTriggerRelationshipEvents(npc, "trade");
  addMissionMetric("npc_trades", 1);
  if (npcTrustValue(npc.id) >= 5) npcAdvanceChainStep(npc, "bond");
  addJournal(npc.name + " \u4e3a\u4f60\u8c03\u914d\u4e86\u4e00\u6279\u4e13\u5c5e\u8d44\u6e90\u3002");
  renderAll();
  renderNpcDialog();
}

function npcActionTrain() {
  const npc = findNpcById(activeNpcDialogId || "");
  if (!npc) return;
  const trust = npcTrustValue(npc.id);
  const cost = Math.max(42, 70 - trust * 2);
  if (state.player.gold < cost) return showToast("\u8bad\u7ec3\u9700\u8981 " + cost + " \u91d1\u5e01");
  state.player.gold -= cost;

  if (npc.questKind === "combat") {
    state.player.attackBonus += 1;
    progressSideQuests("combat", 1);
  } else if (npc.questKind === "artifact") {
    state.player.maxMp += 5;
    state.player.mp = Math.min(state.player.maxMp, state.player.mp + 5);
    progressSideQuests("artifact", 1);
  } else {
    state.player.maxHp += 6;
    state.player.hp = Math.min(state.player.maxHp, state.player.hp + 6);
    progressSideQuests("explore", 1);
  }

  state.talents.points = (state.talents.points || 0) + 1;
  addMissionMetric("npc_training", 1);
  setNpcTrust(npc.id, 1);
  npcTryTriggerRelationshipEvents(npc, "train");
  if (npcTrustValue(npc.id) >= 5) npcAdvanceChainStep(npc, "bond");

  const companion = activeCompanion();
  grantCompanionGrowth("\u534f\u540c\u8bad\u7ec3", 18, 1, { toast: false });
  addJournal(npc.name + " \u4e3a\u4f60\u5b89\u6392\u4e86\u4e13\u9879\u8bad\u7ec3" + (companion ? ("\uff0c" + companion.name + " \u4e5f\u83b7\u5f97\u4e86\u6210\u957f") : ""));
  showToast("\u8bad\u7ec3\u5b8c\u6210\uff0c\u5c5e\u6027 +1");
  renderAll();
  renderNpcDialog();
}

function renderNpcPanel() {
  if (!refs.npcSummary || !refs.npcList) return;
  refreshNpcPopulation(false);

  const chapterId = currentChapterId();
  const chapterChain = npcChapterChainSummary(chapterId);
  const chapterRelation = npcChapterRelationshipSummary(chapterId);
  const chapterRewarded = (state.npcs.chapterRewards || []).includes("chain_reward_" + chapterId);
  const support = chapterSupportSummary(chapterId);
  const list = (state.npcs.activeMap || []).slice().sort((a, b) => npcDistanceToPlayer(a) - npcDistanceToPlayer(b));

  if (!list.length) {
    refs.npcSummary.textContent = "\u9644\u8fd1 NPC \u4f1a\u5728\u63a2\u7d22\u533a\u57df\u5237\u65b0\u3002";
    refs.npcList.innerHTML = "";
    return;
  }

  const nearest = list[0];
  const supportHint = !support.total
    ? "\u672c\u7ae0\u8282\u6682\u65e0\u534f\u4f5c\u884c\u52a8"
    : support.current
      ? ("\u5f53\u524d\u534f\u4f5c\uff1a" + support.current.title + (support.currentStatus?.ok ? "\uff0c\u6761\u4ef6\u5df2\u6ee1\u8db3" : ("\uff0c\u4ecd\u9700\uff1a" + supportPrimaryBlockerText(support.current, support.currentStatus, chapterId))))
      : "\u7ae0\u8282\u534f\u4f5c\u5df2\u5168\u90e8\u5b8c\u6210";

  refs.npcSummary.textContent =
    "\u9644\u8fd1 NPC: " + list.length +
    " | \u6700\u8fd1: " + nearest.name + " (" + Math.round(npcDistanceToPlayer(nearest)) + ")" +
    " | \u5267\u60c5\u7ebf: " + chapterChain.completed + "/" + chapterChain.total +
    " | \u5173\u7cfb\u4e8b\u4ef6: " + chapterRelation.unlocked + "/" + chapterRelation.total +
    (chapterRewarded ? " | \u534f\u4f5c\u5956\u52b1\u5df2\u89e3\u9501" : "") +
    "\n" + supportHint;

  refs.npcList.innerHTML = "";
  list.forEach((npc) => {
    const row = document.createElement("div");
    const trust = npcTrustValue(npc.id);
    const trustLabel = npcTrustLabel(trust);
    const qs = npcQuestStatus(npc);
    const chain = npcChainProgress(npc);
    const rel = npcRelationshipEventStatus(npc);
    const statusText = qs.state === "none"
      ? "\u53ef\u63a5\u59d4\u6258"
      : (qs.state === "active" ? "\u59d4\u6258\u8fdb\u884c\u4e2d" : (qs.state === "turnin" ? "\u53ef\u4ea4\u4ed8\u59d4\u6258" : "\u5df2\u5b8c\u6210"));

    row.className = "quest-item" + (npc.id === state.npcs.selectedId ? " done" : "");
    row.textContent =
      npc.name + " | " + npc.title +
      " | \u597d\u611f " + trust + "/10 (" + trustLabel + ")" +
      " | \u5267\u60c5 " + chain.doneCount + "/3" +
      " | \u5173\u7cfb " + rel.unlocked + "/" + rel.total +
      " | " + statusText +
      " | \u8ddd\u79bb " + Math.round(npcDistanceToPlayer(npc));
    row.addEventListener("click", () => openNpcDialogById(npc.id));
    refs.npcList.appendChild(row);
  });
}

function renderStoryGuideModal() {
  const contentEl = refs.storyGuideContent;
  if (!contentEl) return;
  const pack = getStoryGuideChecklist();
  storyGuideRecommendedAction = getStoryGuidePrimaryAction(pack);

  const lines = [];
  lines.push("\u7ae0\u8282\uff1a" + pack.c.name);
  lines.push("\u63a8\u8350\u884c\u52a8\uff1a" + pack.status.nextAction);

  if (pack.support?.current) {
    lines.push("\u7ae0\u8282\u534f\u4f5c\uff1a" + pack.support.current.title);
    if (pack.support.currentStatus?.ok) {
      lines.push("\u5f53\u524d\u6761\u4ef6\u5df2\u6ee1\u8db3\uff0c\u53ef\u4ee5\u76f4\u63a5\u63a8\u8fdb\u7ae0\u8282\u534f\u4f5c\u3002");
    } else {
      lines.push("\u4ecd\u9700\u51c6\u5907\uff1a" + supportPrimaryBlockerText(pack.support.current, pack.support.currentStatus, pack.c.id));
    }

    supportRequirementRows(pack.support.current, pack.support.currentStatus, pack.c.id).forEach((row) => {
      lines.push((row.done ? "[x] " : "[ ] ") + row.label + "\uff1a" + row.current + "/" + row.target + (row.hint ? (" | " + row.hint) : ""));
    });
  }

  lines.push("");
  lines.push("\u68c0\u67e5\u6e05\u5355\uff1a");
  pack.items.forEach((item) => {
    lines.push((item.done ? "[x] " : "[ ] ") + item.text);
  });
  lines.push("");
  lines.push("\u4e0b\u4e00\u6b65\uff1a" + (storyGuideRecommendedAction ? storyGuideRecommendedAction.label : "\u5f53\u524d\u6ca1\u6709\u53ef\u6267\u884c\u5efa\u8bae"));

  contentEl.textContent = lines.join("\n");

  if (refs.btnStoryGuideAction) {
    refs.btnStoryGuideAction.disabled = !storyGuideRecommendedAction;
    refs.btnStoryGuideAction.textContent = storyGuideRecommendedAction
      ? ("\u6267\u884c\uff1a" + storyGuideRecommendedAction.label)
      : "\u6682\u65e0\u5efa\u8bae";
  }
}

function runStoryGuideAction() {
  if (!storyGuideRecommendedAction) {
    showToast("\u5f53\u524d\u6ca1\u6709\u53ef\u6267\u884c\u7684\u4e0b\u4e00\u6b65");
    return;
  }

  closeModal("modalStoryGuide");
  const id = storyGuideRecommendedAction.id;
  if (id === "btnCommitChoice") return commitChoice();
  if (id === "btnStoryDecision") return openStoryDecisionModal();
  if (id === "btnStartBoss") return startBoss();
  if (id === "btnStrikeBoss") return strikeBoss();
  if (id === "btnExplore") return exploreArea();
  if (id === "btnAdvanceChapter") return advanceChapter();
  if (id === "actionInteractPoi") return interactNearestRegionPoi();
  if (id === "actionOpenRumorNpc" && typeof openCurrentRumorNpc === "function") return openCurrentRumorNpc();

  document.getElementById(id)?.click();
}

function openStoryGuideModal() {
  renderStoryGuideModal();
  openModal("modalStoryGuide");
}

function maybeAutoOpenStoryGuide() {
  const localState = window.FantasyLocalStateRuntime;
  if (localState?.isStoryGuideAutoShown?.()) return;
  if (state.currentView !== "gameplay") return;
  localState?.markStoryGuideAutoShown?.(true);
  setTimeout(() => openStoryGuideModal(), 220);
}
