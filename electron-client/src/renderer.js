const SETTINGS_KEY = "fantasy_legend_settings_v1";

const state = {
    currentView: "mainMenu",
    player: {
        name: "无",
        profession: "warrior",
        level: 1,
        hp: 100,
        mp: 60,
        gold: 100
    },
    world: {
        chapter: "新手村",
        location: "晨风营地",
        day: 1,
        x: 360,
        y: 200,
        step: 16
    },
    selectedProfession: "warrior",
    settings: {
        volume: 70,
        fpsLimit: "60"
    },
    rules: []
};

const refs = {
    viewMainMenu: document.getElementById("viewMainMenu"),
    viewGameplay: document.getElementById("viewGameplay"),
    serviceState: document.getElementById("serviceState"),
    worldMap: document.getElementById("worldMap"),
    playerAvatar: document.getElementById("playerAvatar"),
    playerPos: document.getElementById("playerPos"),
    toast: document.getElementById("toast"),
    inventoryDrawer: document.getElementById("inventoryDrawer"),
    inventoryGrid: document.getElementById("inventoryGrid"),
    modalNewGame: document.getElementById("modalNewGame"),
    modalLoadGame: document.getElementById("modalLoadGame"),
    modalSettings: document.getElementById("modalSettings"),
    saveList: document.getElementById("saveList"),
    inputPlayerName: document.getElementById("inputPlayerName"),
    btnNewGame: document.getElementById("btnNewGame"),
    btnLoadGame: document.getElementById("btnLoadGame"),
    btnSettings: document.getElementById("btnSettings"),
    btnExit: document.getElementById("btnExit"),
    btnInventory: document.getElementById("btnInventory"),
    btnQuest: document.getElementById("btnQuest"),
    btnSkillTree: document.getElementById("btnSkillTree"),
    btnSave: document.getElementById("btnSave"),
    btnBackMenu: document.getElementById("btnBackMenu"),
    closeInventory: document.getElementById("closeInventory"),
    createCharacter: document.getElementById("createCharacter"),
    saveSettings: document.getElementById("saveSettings"),
    volumeRange: document.getElementById("volumeRange"),
    fpsLimit: document.getElementById("fpsLimit"),
    inputAttacker: document.getElementById("inputAttacker"),
    inputDefender: document.getElementById("inputDefender"),
    inputSkill: document.getElementById("inputSkill"),
    btnRunDamage: document.getElementById("btnRunDamage"),
    btnRunAI: document.getElementById("btnRunAI"),
    btnReloadRules: document.getElementById("btnReloadRules"),
    btnSyncState: document.getElementById("btnSyncState"),
    btnTriggerStory: document.getElementById("btnTriggerStory"),
    btnTriggerCombat: document.getElementById("btnTriggerCombat"),
    combatResult: document.getElementById("combatResult"),
    ruleList: document.getElementById("ruleList"),
    eventLog: document.getElementById("eventLog"),
    serverVars: document.getElementById("serverVars"),
    serverFlags: document.getElementById("serverFlags"),
    debugWorldState: document.getElementById("debugWorldState"),
    btnLoadStateJson: document.getElementById("btnLoadStateJson"),
    btnApplyStateJson: document.getElementById("btnApplyStateJson")
};

function appendLog(message) {
    const now = new Date();
    const stamp = `${now.getHours().toString().padStart(2, "0")}:${now.getMinutes().toString().padStart(2, "0")}:${now.getSeconds().toString().padStart(2, "0")}`;
    const line = `[${stamp}] ${message}`;
    refs.eventLog.textContent = refs.eventLog.textContent
        ? `${line}\n${refs.eventLog.textContent}`
        : line;
}

function showToast(message) {
    refs.toast.textContent = message;
    refs.toast.classList.add("show");
    window.clearTimeout(showToast.timer);
    showToast.timer = window.setTimeout(() => refs.toast.classList.remove("show"), 1800);
}

function setView(viewName) {
    state.currentView = viewName;
    refs.viewMainMenu.classList.toggle("view-active", viewName === "mainMenu");
    refs.viewGameplay.classList.toggle("view-active", viewName === "gameplay");
}

function openModal(id) {
    document.getElementById(id).classList.remove("hidden");
}

function closeModal(id) {
    document.getElementById(id).classList.add("hidden");
}

function updateHUD() {
    document.getElementById("playerName").textContent = `名称: ${state.player.name}`;
    const professionMap = { warrior: "战士", mage: "法师", archer: "弓箭手" };
    document.getElementById("playerProfession").textContent = `职业: ${professionMap[state.player.profession] || "无"}`;
    document.getElementById("playerLevel").textContent = `等级: ${state.player.level}`;
    document.getElementById("playerHP").textContent = `生命: ${state.player.hp} / 100`;
    document.getElementById("playerMP").textContent = `法力: ${state.player.mp} / 60`;
    document.getElementById("gameChapter").textContent = `章节: ${state.world.chapter}`;
    document.getElementById("gameLocation").textContent = `地点: ${state.world.location}`;
    document.getElementById("gameTime").textContent = `时间: Day ${state.world.day}`;
    document.getElementById("gameGold").textContent = `金币: ${state.player.gold}`;
    refs.playerPos.textContent = `坐标: (${state.world.x}, ${state.world.y})`;
    refs.playerAvatar.style.left = `${state.world.x}px`;
    refs.playerAvatar.style.top = `${state.world.y}px`;
}

function loadSettings() {
    try {
        const raw = localStorage.getItem(SETTINGS_KEY);
        if (!raw) {
            refs.volumeRange.value = state.settings.volume;
            refs.fpsLimit.value = state.settings.fpsLimit;
            return;
        }

        const parsed = JSON.parse(raw);
        state.settings.volume = Number(parsed.volume || state.settings.volume);
        state.settings.fpsLimit = String(parsed.fpsLimit || state.settings.fpsLimit);
        refs.volumeRange.value = String(state.settings.volume);
        refs.fpsLimit.value = state.settings.fpsLimit;
    } catch {
        refs.volumeRange.value = String(state.settings.volume);
        refs.fpsLimit.value = state.settings.fpsLimit;
    }
}

function persistSettings() {
    const data = {
        volume: Number(refs.volumeRange.value),
        fpsLimit: refs.fpsLimit.value
    };
    localStorage.setItem(SETTINGS_KEY, JSON.stringify(data));
    state.settings.volume = data.volume;
    state.settings.fpsLimit = data.fpsLimit;
}

function applyStateJson(stateJson) {
    if (!stateJson) {
        return;
    }

    try {
        const parsed = JSON.parse(stateJson);
        if (parsed.global_variables) {
            const vars = parsed.global_variables;
            if (typeof vars.player_level === "number") {
                state.player.level = vars.player_level;
            }
            if (typeof vars.player_exp === "number") {
                state.player.gold = vars.player_exp;
            }
            if (typeof vars.story_progress === "number") {
                state.world.day = Math.max(1, vars.story_progress + 1);
            }
        }

        if (typeof parsed.chapter === "string") {
            state.world.chapter = parsed.chapter;
        }
        if (typeof parsed.location === "string") {
            state.world.location = parsed.location;
        }
        if (typeof parsed.day === "number") {
            state.world.day = Math.max(1, parsed.day);
        }

        if (parsed.player && typeof parsed.player === "object") {
            if (typeof parsed.player.name === "string" && parsed.player.name) {
                state.player.name = parsed.player.name;
            }
            if (typeof parsed.player.profession === "string") {
                state.player.profession = parsed.player.profession;
            }
            if (typeof parsed.player.hp === "number") {
                state.player.hp = parsed.player.hp;
            }
            if (typeof parsed.player.mp === "number") {
                state.player.mp = parsed.player.mp;
            }
            if (typeof parsed.player.gold === "number") {
                state.player.gold = parsed.player.gold;
            }
        }

        if (parsed.position && typeof parsed.position === "object") {
            if (typeof parsed.position.x === "number") {
                state.world.x = parsed.position.x;
            }
            if (typeof parsed.position.y === "number") {
                state.world.y = parsed.position.y;
            }
        }

        if (Array.isArray(parsed.active_events) && parsed.active_events.length > 0) {
            appendLog(`Active events: ${parsed.active_events.join(", ")}`);
        }
    } catch {
        appendLog("Failed to parse world state json");
    }
}

function renderRules() {
    refs.ruleList.innerHTML = "";
    if (state.rules.length === 0) {
        refs.ruleList.textContent = "暂无规则";
        return;
    }

    const topRules = state.rules.slice(0, 8);
    topRules.forEach((rule) => {
        const item = document.createElement("div");
        item.className = "rule-item";
        item.textContent = `[${rule.category}] ${rule.rule_id}`;
        refs.ruleList.appendChild(item);
    });
}

function renderKeyValueList(container, mapObj) {
    container.innerHTML = "";
    const entries = Object.entries(mapObj || {});

    if (entries.length === 0) {
        container.textContent = "无数据";
        return;
    }

    entries.slice(0, 12).forEach(([key, value]) => {
        const item = document.createElement("div");
        item.className = "rule-item";
        item.textContent = `${key}: ${value}`;
        container.appendChild(item);
    });
}

async function syncServerState() {
    try {
        const [globalVarsResp, flagsResp] = await Promise.all([
            window.gameApi.queryGameState({ query_type: "global_vars", entity_id: 0 }),
            window.gameApi.queryGameState({ query_type: "flags", entity_id: 0 })
        ]);

        const globalVars = globalVarsResp.world_state?.global_variables || {};
        const flags = flagsResp.world_state?.world_flags || {};

        renderKeyValueList(refs.serverVars, globalVars);
        renderKeyValueList(refs.serverFlags, flags);
        appendLog(`Synced state snapshot: ${Object.keys(globalVars).length} vars, ${Object.keys(flags).length} flags`);
    } catch (error) {
        appendLog(`Sync state failed: ${error.details || error.message}`);
        showToast(`状态同步失败: ${error.details || error.message}`);
    }
}

async function loadCurrentStateJson() {
    try {
        const response = await window.gameApi.queryGameState({ query_type: "world", entity_id: 0 });
        refs.debugWorldState.value = response.state_json || "{}";
        appendLog("Loaded world_state_json into debug panel");
    } catch (error) {
        showToast(`读取状态失败: ${error.details || error.message}`);
        appendLog(`Load state json failed: ${error.details || error.message}`);
    }
}

async function applyDebugWorldState() {
    const raw = (refs.debugWorldState.value || "").trim();
    if (!raw) {
        showToast("请输入或读取状态JSON");
        return;
    }

    try {
        JSON.parse(raw);
    } catch {
        showToast("JSON 格式错误");
        return;
    }

    try {
        await window.gameApi.updateWorldState({ world_state_json: raw });
        applyStateJson(raw);
        await syncServerState();
        updateHUD();
        appendLog("Applied debug world_state_json");
        showToast("状态下发成功");
    } catch (error) {
        showToast(`状态下发失败: ${error.details || error.message}`);
        appendLog(`Apply state json failed: ${error.details || error.message}`);
    }
}
function applyBootstrapData(payload) {
    if (!payload) {
        return;
    }

    const health = payload.health;
    if (health) {
        const alg = health.algorithm.ok ? "Algorithm OK" : `Algorithm ERR: ${health.algorithm.error}`;
        const stg = health.strategy.ok ? "Strategy OK" : `Strategy ERR: ${health.strategy.error}`;
        refs.serviceState.textContent = `${alg} | ${stg}`;
    }

    if (payload.state?.state_json) {
        applyStateJson(payload.state.state_json);
        refs.debugWorldState.value = payload.state.state_json;
    }

    if (payload.rules && Array.isArray(payload.rules.rules)) {
        state.rules = payload.rules.rules;
        renderRules();
        appendLog(`Loaded rules: ${payload.rules.rules.length}`);
    }

    updateHUD();
}

async function reloadRules() {
    try {
        const response = await window.gameApi.getGameRules({
            category: "all",
            active_only: true
        });
        state.rules = Array.isArray(response.rules) ? response.rules : [];
        renderRules();
        showToast(`规则刷新完成: ${state.rules.length}`);
        appendLog(`Rules reloaded: ${state.rules.length}`);
    } catch (error) {
        showToast(`规则刷新失败: ${error.details || error.message}`);
    }
}

async function runDamageSimulation() {
    const attacker = Number(refs.inputAttacker.value || 1);
    const defender = Number(refs.inputDefender.value || 2);
    const skill = Number(refs.inputSkill.value || 1);

    try {
        const result = await window.gameApi.calculateDamage({
            attacker_id: attacker,
            defender_id: defender,
            skill_id: skill
        });

        refs.combatResult.textContent = `伤害 ${result.damage} | 暴击 ${result.is_critical ? "是" : "否"} | 效果 ${result.effect}`;
        appendLog(`Damage sim: ${attacker}->${defender} skill ${skill}, dmg=${result.damage}`);
    } catch (error) {
        refs.combatResult.textContent = `计算失败: ${error.details || error.message}`;
        appendLog(`Damage sim failed: ${error.details || error.message}`);
    }
}

async function runAIDecision() {
    try {
        const result = await window.gameApi.aiDecision({
            npc_id: Number(refs.inputDefender.value || 2),
            context: [
                state.world.x,
                state.player.hp,
                0,
                state.player.level
            ]
        });

        refs.combatResult.textContent = `AI 行为 ${result.action_id} | 置信度 ${Number(result.confidence || 0).toFixed(2)} | ${result.description}`;
        appendLog(`AI decision: action=${result.action_id}, confidence=${result.confidence}`);
    } catch (error) {
        refs.combatResult.textContent = `AI 失败: ${error.details || error.message}`;
        appendLog(`AI decision failed: ${error.details || error.message}`);
    }
}

async function triggerEvent(eventId, params) {
    try {
        const result = await window.gameApi.triggerEvent({ event_id: eventId, params });
        appendLog(`Event ${eventId} => triggered=${result.triggered}`);
        showToast(`事件 ${eventId} 已触发`);

        if (result.result_json) {
            appendLog(`Event payload: ${result.result_json}`);
        }

        await syncServerState();
    } catch (error) {
        appendLog(`Event ${eventId} failed: ${error.details || error.message}`);
        showToast(`事件触发失败: ${error.details || error.message}`);
    }
}

function buildWorldTiles() {
    const tileSize = 48;
    const cols = Math.ceil(refs.worldMap.clientWidth / tileSize);
    const rows = Math.ceil(refs.worldMap.clientHeight / tileSize);

    const frag = document.createDocumentFragment();
    for (let r = 0; r < rows; r += 1) {
        const row = document.createElement("div");
        row.className = "world-row";
        for (let c = 0; c < cols; c += 1) {
            const tile = document.createElement("div");
            tile.className = "world-tile";
            tile.style.backgroundColor = (c + r) % 2 === 0 ? "#2f5a3d" : "#366947";
            row.appendChild(tile);
        }
        frag.appendChild(row);
    }

    refs.worldMap.prepend(frag);

    [
        { x: 120, y: 100, size: 42, color: "#2f7a3a" },
        { x: 300, y: 240, size: 32, color: "#8d8d8d" },
        { x: 620, y: 160, size: 50, color: "#307f48" },
        { x: 520, y: 320, size: 30, color: "#848484" }
    ].forEach((node) => {
        const div = document.createElement("div");
        div.className = "map-ornament";
        div.style.left = `${node.x}px`;
        div.style.top = `${node.y}px`;
        div.style.width = `${node.size}px`;
        div.style.height = `${node.size}px`;
        div.style.background = node.color;
        refs.worldMap.appendChild(div);
    });
}

function buildInventory() {
    refs.inventoryGrid.innerHTML = "";
    for (let i = 0; i < 24; i += 1) {
        const slot = document.createElement("div");
        slot.className = "inventory-item";
        slot.textContent = i < 6 ? `物品 ${i + 1}` : "空";
        refs.inventoryGrid.appendChild(slot);
    }
}

function buildSaveSlots() {
    refs.saveList.innerHTML = "";
    for (let i = 0; i < 10; i += 1) {
        const item = document.createElement("div");
        item.className = "save-item";
        item.innerHTML = `<span>存档 ${i + 1}</span><button data-slot="${i}">读取</button>`;
        refs.saveList.appendChild(item);
    }

    refs.saveList.addEventListener("click", async (event) => {
        const target = event.target;
        if (!(target instanceof HTMLButtonElement) || !target.dataset.slot) {
            return;
        }

        const slot = Number(target.dataset.slot);
        try {
            const response = await window.gameApi.queryGameState({ query_type: "player", entity_id: slot });
            applyStateJson(response.state_json);
            showToast(`已加载存档 ${slot + 1}`);
            setView("gameplay");
            closeModal("modalLoadGame");
            updateHUD();
            await syncServerState();
            appendLog(`Loaded save slot: ${slot + 1}`);
        } catch (error) {
            showToast(`读取失败: ${error.details || error.message}`);
        }
    });
}

function movePlayer(dx, dy) {
    if (state.currentView !== "gameplay") {
        return;
    }

    const maxX = refs.worldMap.clientWidth - refs.playerAvatar.clientWidth;
    const maxY = refs.worldMap.clientHeight - refs.playerAvatar.clientHeight;
    state.world.x = Math.max(0, Math.min(maxX, state.world.x + dx));
    state.world.y = Math.max(0, Math.min(maxY, state.world.y + dy));
    updateHUD();
}

async function bootstrapFromBackend() {
    try {
        const payload = await window.gameApi.bootstrapGame();
        applyBootstrapData(payload);
        await syncServerState();
        appendLog("Bootstrap completed");
    } catch (error) {
        refs.serviceState.textContent = `Bootstrap failed: ${error.message}`;
        appendLog(`Bootstrap failed: ${error.message}`);
    }
}

function bindEvents() {
    refs.btnNewGame.addEventListener("click", () => openModal("modalNewGame"));
    refs.btnLoadGame.addEventListener("click", () => openModal("modalLoadGame"));
    refs.btnSettings.addEventListener("click", () => openModal("modalSettings"));
    refs.btnExit.addEventListener("click", () => window.close());

    refs.btnInventory.addEventListener("click", () => refs.inventoryDrawer.classList.add("open"));
    refs.closeInventory.addEventListener("click", () => refs.inventoryDrawer.classList.remove("open"));

    refs.btnQuest.addEventListener("click", async () => {
        await reloadRules();
        showToast("任务与规则已更新");
    });

    refs.btnSkillTree.addEventListener("click", async () => {
        try {
            const response = await window.gameApi.getSkillTree({
                character_id: 1,
                learned_skills: [1, 2]
            });
            showToast(`技能节点: ${response.skills.length}`);
            appendLog(`Skill tree loaded: ${response.skills.length}`);
        } catch (error) {
            showToast(`技能树请求失败: ${error.details || error.message}`);
        }
    });

    refs.btnSave.addEventListener("click", async () => {
        try {
            const worldState = {
                global_variables: {
                    player_level: state.player.level,
                    player_exp: state.player.gold,
                    story_progress: Math.max(0, state.world.day - 1)
                },
                world_flags: {
                    in_combat: false,
                    tutorial_complete: true
                },
                active_events: [],
                chapter: state.world.chapter,
                location: state.world.location,
                day: state.world.day,
                player: state.player,
                position: { x: state.world.x, y: state.world.y },
                settings: state.settings
            };

            await window.gameApi.updateWorldState({
                world_state_json: JSON.stringify(worldState)
            });
            await syncServerState();
            showToast("保存成功");
            appendLog("World state saved");
        } catch (error) {
            showToast(`保存失败: ${error.details || error.message}`);
        }
    });

    refs.btnBackMenu.addEventListener("click", () => setView("mainMenu"));

    refs.createCharacter.addEventListener("click", () => {
        const name = refs.inputPlayerName.value.trim();
        if (!name) {
            showToast("请输入角色名");
            return;
        }

        state.player.name = name;
        state.player.profession = state.selectedProfession;
        state.player.level = 1;
        state.world.day = 1;
        updateHUD();
        closeModal("modalNewGame");
        setView("gameplay");
        showToast(`欢迎，${name}`);
        appendLog(`New game created: ${name}`);
    });

    refs.saveSettings.addEventListener("click", () => {
        persistSettings();
        closeModal("modalSettings");
        showToast("设置已保存");
        appendLog(`Settings saved: volume=${state.settings.volume}, fps=${state.settings.fpsLimit}`);
    });

    refs.btnRunDamage.addEventListener("click", runDamageSimulation);
    refs.btnRunAI.addEventListener("click", runAIDecision);
    refs.btnReloadRules.addEventListener("click", reloadRules);
    refs.btnSyncState.addEventListener("click", syncServerState);
    refs.btnLoadStateJson.addEventListener("click", loadCurrentStateJson);
    refs.btnApplyStateJson.addEventListener("click", applyDebugWorldState);
    refs.btnTriggerStory.addEventListener("click", () => triggerEvent(1001, [state.world.day]));
    refs.btnTriggerCombat.addEventListener("click", () => triggerEvent(2001, [50]));

    document.querySelectorAll("[data-close]").forEach((button) => {
        button.addEventListener("click", () => {
            closeModal(button.dataset.close);
        });
    });

    document.querySelectorAll(".profession").forEach((button) => {
        button.addEventListener("click", () => {
            document.querySelectorAll(".profession").forEach((node) => node.classList.remove("selected"));
            button.classList.add("selected");
            state.selectedProfession = button.dataset.profession || "warrior";
        });
    });

    window.addEventListener("keydown", (event) => {
        switch (event.key.toLowerCase()) {
            case "w":
            case "arrowup":
                movePlayer(0, -state.world.step);
                break;
            case "s":
            case "arrowdown":
                movePlayer(0, state.world.step);
                break;
            case "a":
            case "arrowleft":
                movePlayer(-state.world.step, 0);
                break;
            case "d":
            case "arrowright":
                movePlayer(state.world.step, 0);
                break;
            case "escape":
                setView("mainMenu");
                break;
            case "i":
                refs.inventoryDrawer.classList.add("open");
                break;
            case "k":
                refs.btnSkillTree.click();
                break;
            case "q":
                refs.btnQuest.click();
                break;
            case "f5":
                event.preventDefault();
                refs.btnSave.click();
                break;
            default:
                break;
        }
    });
}

function bootstrap() {
    loadSettings();
    buildWorldTiles();
    buildInventory();
    buildSaveSlots();
    bindEvents();
    updateHUD();
    bootstrapFromBackend();
}

bootstrap();





