const {
  CHAPTERS,
  JOBS,
  SKILL_LIBRARY,
  COMPANION_ARCHETYPES,
  STORY_EVENTS,
  CHAPTER_STORYLINES,
  CHAPTER_ARCS,
  BOSS_PROFILES,
  CHAPTER_DECISIONS,
  CAMPFIRE_SCENES,
  NPC_PROFILES,
  NPC_QUEST_TEMPLATES,
  NPC_STORY_CHAINS,
  NPC_RELATIONSHIP_EVENTS,
  NPC_TRADE_PROFILES,
  CHAPTER_MISSIONS,
  CHAPTER_SUPPORT_OPERATIONS,
  REGION_POI_LIBRARY
} = window.FantasyGameData || {};

if (!window.FantasyGameData) {
  throw new Error("FantasyGameData module failed to load.");
}

const {
  applyChineseStoryContent,
  applyChineseUiText
} = window.FantasyUiTextModule || {};

if (!window.FantasyUiTextModule) {
  throw new Error("FantasyUiTextModule failed to load.");
}

const {
  applyDecisionReward,
  buildChapterSummaryReport,
  buildDecisionImpactDetails,
  evaluateEndingNarrative,
  exportChapterSummary,
  exportEndingReview,
  exportStoryReplay,
  getPendingChapterDecision,
  openChapterSummaryModal,
  openEndingReviewModal,
  openStoryReplayModal,
  pushStoryCard,
  renderChapterSummary,
  renderStoryReplay
} = window.FantasyStoryArchiveModule || {};

if (!window.FantasyStoryArchiveModule) {
  throw new Error("FantasyStoryArchiveModule failed to load.");
}

const formatRewardInline = window.FantasyStoryArchiveModule?.formatRewardInline;

const {
  createLocalStateModule
} = window.FantasyLocalStateModule || {};

if (!window.FantasyLocalStateModule) {
  throw new Error("FantasyLocalStateModule failed to load.");
}

const {
  createGameplayRuntimeModule
} = window.FantasyGameplayRuntimeModule || {};

if (!window.FantasyGameplayRuntimeModule) {
  throw new Error("FantasyGameplayRuntimeModule failed to load.");
}

const {
  createProgressionRuntimeModule
} = window.FantasyProgressionRuntimeModule || {};

if (!window.FantasyProgressionRuntimeModule) {
  throw new Error("FantasyProgressionRuntimeModule failed to load.");
}

const {
  createStoryRuntimeModule
} = window.FantasyStoryRuntimeModule || {};

if (!window.FantasyStoryRuntimeModule) {
  throw new Error("FantasyStoryRuntimeModule failed to load.");
}

const {
  createUiPanelsModule
} = window.FantasyUiPanelsModule || {};

if (!window.FantasyUiPanelsModule) {
  throw new Error("FantasyUiPanelsModule failed to load.");
}


const {
  createBackendSyncModule
} = window.FantasyBackendSyncModule || {};

if (!window.FantasyBackendSyncModule) {
  throw new Error("FantasyBackendSyncModule failed to load.");
}

const {
  createAppShellModule
} = window.FantasyAppShellModule || {};

if (!window.FantasyAppShellModule) {
  throw new Error("FantasyAppShellModule failed to load.");
}


const {
  createAppCoreModule
} = window.FantasyAppCoreModule || {};

if (!window.FantasyAppCoreModule) {
  throw new Error("FantasyAppCoreModule failed to load.");
}


const state = {
  currentView: "mainMenu",
  player: {
    name: "\u65e0\u540d\u5192\u9669\u8005",
    profession: "warrior",
    level: 1,
    exp: 0,
    hp: 130,
    mp: 50,
    maxHp: 130,
    maxMp: 50,
    gold: 100,
    potions: 2,
    baseAttack: 18,
    baseDefense: 12,
    attackBonus: 0,
    defenseBonus: 0,
    gear: { weapon: "\u8bad\u7ec3\u77ed\u5251", armor: "\u76ae\u7532" }
  },
  world: { chapter: CHAPTERS[0].name, location: "\u6668\u98ce\u5e73\u539f", day: 1, x: 280, y: 180, step: 18, regionX: 0, regionY: 0 },
  story: { chapterIndex: 0, progress: { explore: 0, artifact: 0, boss: 0 }, route: "balanced", choice: "" },
  combat: { active: false, bossName: "", hp: 0, maxHp: 0, timeline: [] },
  content: { artifactShards: 0, forgedRelics: 0, shop: [] },
  settings: { volume: 70, fpsLimit: "60" },
  selectedProfession: "warrior",
  rules: [],
  factions: { wardens: 0, arcanum: 0, freeguild: 0 },
  sideQuests: [],
  journal: [],
  party: { roster: [], activeId: "", nextId: 1 },
  talents: { offense: 0, guard: 0, support: 0, points: 0 },
  skillLoadout: ["quick_strike", "guard_break", "aether_burst"],
  storyRuntime: { beats: 0, completedSideQuests: 0, decisions: {}, arc: {}, storyCards: [], chapterReports: [], pinnedStoryCardIds: [], collapsedReplayChapters: [], missionProgress: {}, missionRewards: [], poiVisited: [] },
  projects: { built: 0, operationsCompleted: [] },
  npcs: { activeMap: [], selectedId: "", trust: {}, claimedQuestRewards: [], spawnKey: "", chainFlags: {}, chapterRewards: [], relationshipEvents: [] }
};

const ids = [
  "viewMainMenu","viewGameplay","serviceState","worldMap","playerAvatar","playerPos","toast","inventoryDrawer","inventoryGrid",
  "modalNewGame","modalLoadGame","modalSettings","modalRuleDiff","saveList","inputPlayerName","btnNewGame","btnLoadGame","btnSettings","btnExit",
  "btnInventory","btnQuest","btnSkillTree","btnSave","btnBackMenu","closeInventory","createCharacter","saveSettings","volumeRange","fpsLimit",
  "inputAttacker","inputDefender","inputSkill","btnRunDamage","btnRunAI","btnReloadRules","btnSyncState","btnTriggerStory","btnTriggerCombat",
  "combatResult","ruleList","eventLog","serverVars","serverFlags","debugWorldState","btnLoadStateJson","btnApplyStateJson",
  "eventRuleRows","btnAddEventRule","btnApplyEventRules","btnFetchEventRules","btnExportEventRules","btnImportEventRules","inputEventRulesFile",
  "eventRulePreview","eventRuleVersionSelect","btnSaveRuleVersion","btnApplyRuleVersion","btnRollbackRuleVersion","btnPreviewRuleDiff","ruleDiffContent","confirmRuleDiff",
  "playerExp","playerPotion","storySummary","questList","btnExplore","btnRest","btnUsePotion","btnAdvanceChapter","storyChoiceSelect","btnCommitChoice","choiceSummary",
  "partyList","btnRecruitCompanion","companionSelect","btnTalentInfo","btnTalentOffense","btnTalentGuard","btnTalentSupport","btnTalentAuto",
  "skillSelect","btnCastSkill","btnCompanionSkill","btnResetBuild","skillSummary","btnStartBoss","btnStrikeBoss","bossStatus","bossTimeline","bossTurnInfo","btnClearBossTimeline",
  "sideQuestList","journalLog","factionReputation","projectSummary","btnRefreshSideQuests","btnCampfireStory","btnBuildProject","shopItemSelect","shopSummary","btnRefreshShop","btnBuyShopItem","btnForgeRelic","forgeSummary","btnViewEndingReview","modalEndingReview","endingReviewContent","btnExportEndingTxt","btnExportEndingJson","btnStoryDecision","btnStoryReplay","btnChapterSummary","btnStoryGuide","modalStoryGuide","storyGuideContent","btnStoryGuideAction","btnExportStoryReplayTxt","btnExportStoryReplayJson","btnExportChapterSummaryTxt","btnExportChapterSummaryJson","selectStoryReplayType","inputStoryReplaySearch","toggleStoryPinnedOnly","toggleStoryGroupByChapter","toggleChapterCompare","compareChapterA","compareChapterB","compareChapterResult","chapterTrendContent","modalStoryDecision","storyDecisionTitle","storyDecisionDesc","storyDecisionOptions","modalStoryReplay","storyReplayList","modalChapterSummary","chapterSummaryContent","btnOpenNpcDialog","npcSummary","npcList","modalNpcDialog","npcDialogTitle","npcDialogBody","npcActionSummary","btnNpcTalk","btnNpcQuest","btnNpcTrade","btnNpcTrain","btnNpcGift"
];

const refs = Object.fromEntries(ids.map((id) => [id, document.getElementById(id)]));

const MAX_LOG_LINES = 140;
const recentLogs = [];
let storyGuideRecommendedAction = null;
let activeNpcDialogId = "";
function safeOn(el, evt, fn) { if (el) el.addEventListener(evt, fn); }
function setTextIfChanged(el, text) {
  if (!el) return;
  if (el.textContent !== text) el.textContent = text;
}
function showToast(msg) { if (!refs.toast) return; refs.toast.textContent = msg; refs.toast.classList.add("show"); clearTimeout(showToast.t); showToast.t = setTimeout(() => refs.toast.classList.remove("show"), 1800); }

function appendLog(msg) {
  if (!refs.eventLog) return;
  const t = new Date().toLocaleTimeString("zh-CN", { hour12: false });
  const line = `[${t}] ${msg}`;
  recentLogs.unshift(line);
  if (recentLogs.length > MAX_LOG_LINES) recentLogs.length = MAX_LOG_LINES;
  refs.eventLog.textContent = recentLogs.join("\n");
}

const localState = createLocalStateModule({
  state,
  refs,
  showToast,
  renderAll,
  setView,
  closeModal,
  storage: window.localStorage
});

window.FantasyLocalStateRuntime = localState;

const backendSync = createBackendSyncModule({
  state,
  refs,
  appendLog,
  showToast,
  storage: window.localStorage
});

const gameplayRuntime = createGameplayRuntimeModule({
  state,
  refs,
  showToast,
  appendLog,
  renderAll,
  levelUpIfNeeded
});

window.FantasyGameplayRuntime = gameplayRuntime;

const {
  addJournal,
  applyReputation,
  buildInventory,
  buildShop,
  buyShopItem,
  forgeRelic,
  gain,
  restAtCamp,
  usePotion
} = gameplayRuntime;

let uiPanelsRuntime = null;
let appCoreRuntime = null;

const progressionRuntime = createProgressionRuntimeModule({
  state,
  refs,
  showToast,
  renderAll,
  renderPanels: () => uiPanelsRuntime?.renderPanels?.(),
  ensureGameplayState,
  currentChapterId,
  chapter,
  addJournal,
  applyReputation,
  gain,
  applyDecisionReward,
  pushStoryCard,
  formatRewardInline,
  regionPoiSummary,
  npcChapterChainSummary,
  npcChapterRelationshipSummary,
  openStoryGuideModal,
  openNearestNpcDialog
});

window.FantasyProgressionRuntime = progressionRuntime;

const {
  activeCompanion,
  activeCompanionSummary,
  addMissionMetric,
  chapterMissionSummary,
  chapterSupportSummary,
  checkChapterMissionRewards,
  companionPower,
  completeSupportOperation,
  ensureCompanionRosterState,
  grantCompanionGrowth,
  missionDone,
  missionMetricValue,
  progressSideQuests,
  recruitCompanion,
  refreshSideQuests,
  renderSupportTracker,
  supportPrimaryBlockerText
} = progressionRuntime;

uiPanelsRuntime = createUiPanelsModule({
  state,
  refs,
  chapter,
  currentChapterId,
  ensureGameplayState,
  getStoryProgressStatus,
  getChapterArcState,
  getCurrentArcNode,
  chapterMissionSummary,
  missionDone,
  missionMetricValue,
  chapterSupportSummary,
  supportPrimaryBlockerText,
  activeCompanion,
  regionPoiSummary,
  renderSupportTracker,
  renderPartyAndSkills,
  renderNpcPanel
});

window.FantasyUiPanelsRuntime = uiPanelsRuntime;

const {
  renderPanels,
  renderStoryAndQuests
} = uiPanelsRuntime;

const storyRuntime = createStoryRuntimeModule({
  state,
  refs,
  showToast,
  renderAll,
  openModal,
  closeModal,
  ensureGameplayState,
  currentChapterId,
  chapter,
  addJournal,
  applyReputation,
  gain,
  addMissionMetric,
  progressSideQuests,
  refreshSideQuests,
  pushStoryCard,
  getPendingChapterDecision,
  buildDecisionImpactDetails,
  applyDecisionReward,
  buildChapterSummaryReport,
  evaluateEndingNarrative,
  openEndingReviewModal,
  getStoryProgressStatus
});

window.FantasyStoryRuntime = storyRuntime;

const {
  advanceChapter,
  checkDecisionRequirement,
  commitChoice,
  exploreArea,
  getChapterArcState,
  getCurrentArcNode,
  openStoryDecisionModal,
  triggerCombatEvent,
  triggerStoryEvent
} = storyRuntime;

appCoreRuntime = createAppCoreModule({
  state,
  refs,
  chapters: CHAPTERS,
  jobs: JOBS,
  chapterArcs: CHAPTER_ARCS,
  showToast,
  setTextIfChanged,
  ensureCompanionRosterState,
  checkChapterMissionRewards,
  getPendingChapterDecision,
  chapterMissionSummary,
  getChapterArcState,
  updateRegionName,
  updatePositionHUD,
  updateAvatarPlacement,
  refreshAvatarEnvironmentFx,
  refreshWorldMapLayout,
  maybeAutoOpenStoryGuide,
  renderStoryGuideModal,
  renderStoryAndQuests,
  renderBoss,
  renderPanels,
  buildInventory
});

window.FantasyAppCoreRuntime = appCoreRuntime;

function setView(v) { return appCoreRuntime?.setView(v); }
function openModal(id) { return appCoreRuntime?.openModal(id); }
function closeModal(id) { return appCoreRuntime?.closeModal(id); }
function chapter() { return appCoreRuntime?.chapter() || CHAPTERS[0]; }
function professionLabel() { return appCoreRuntime?.professionLabel() || "鏈煡"; }
function levelUpIfNeeded() { return appCoreRuntime?.levelUpIfNeeded(); }
function updateHUD() { return appCoreRuntime?.updateHUD(); }
function getMainObjectiveHint(c) { return appCoreRuntime?.getMainObjectiveHint(c) || ""; }
function getStoryProgressStatus(c = chapter()) { return appCoreRuntime?.getStoryProgressStatus(c) || { canAdvance: false, blockers: [], nextAction: "", objectiveRemaining: 0, sideNeed: 0, arcRemaining: 0, mission: { total: 0, done: 0, remaining: 0, next: null }, pendingDecision: null, mainKey: "", progress: 0, target: 0 }; }
function currentChapterId() { return appCoreRuntime?.currentChapterId() || chapter().id; }
function ensureGameplayState() { return appCoreRuntime?.ensureGameplayState(); }
function renderAll() { return appCoreRuntime?.renderAll(); }

const appShellRuntime = createAppShellModule({
  state,
  refs,
  jobs: JOBS,
  localState,
  backendSync,
  safeOn,
  showToast,
  setView,
  openModal,
  closeModal,
  ensureGameplayState,
  currentChapterId,
  renderAll,
  buildWorldTiles,
  buildInventory,
  buildShop,
  refreshSideQuests,
  refreshWorldMapLayout,
  applyChineseStoryContent,
  applyChineseUiText,
  renderStoryReplay,
  renderChapterSummary,
  storyTextData: {
    STORY_EVENTS,
    CAMPFIRE_SCENES,
    CHAPTER_ARCS,
    NPC_RELATIONSHIP_EVENTS
  },
  actions: {
    activeCompanion,
    addJournal,
    advanceChapter,
    applyTalent,
    autoAllocateTalents,
    buyShopItem,
    castCompanionSkill,
    castSelectedSkill,
    clearBossTimeline,
    commitChoice,
    completeSupportOperation,
    exploreArea,
    exportChapterSummary,
    exportEndingReview,
    exportStoryReplay,
    forgeRelic,
    gain,
    getCampfirePool: (chapterId) => CAMPFIRE_SCENES[chapterId] || CAMPFIRE_SCENES.camp,
    grantCompanionGrowth,
    interactNearestRegionPoi,
    movePlayer,
    npcActionGift,
    npcActionQuest,
    npcActionTalk,
    npcActionTrade,
    npcActionTrain,
    openChapterSummaryModal,
    openEndingReviewModal,
    openNearestNpcDialog,
    openStoryDecisionModal,
    openStoryGuideModal,
    openStoryReplayModal,
    recruitCompanion,
    renderPanels,
    resetBuild,
    restAtCamp,
    runAIDecision,
    runDamageSimulation,
    runStoryGuideAction,
    showTalentSummary,
    startBoss,
    strikeBoss,
    teleportPlayerTo,
    triggerCombatEvent,
    triggerStoryEvent,
    usePotion
  }
});

const {
  bindEvents,
  bootstrap
} = appShellRuntime;

bootstrap();
