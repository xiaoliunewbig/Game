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
    selectedProfession: "warrior"
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
    saveSettings: document.getElementById("saveSettings")
};

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
            await window.gameApi.queryGameState({ query_type: "player", entity_id: slot });
            showToast(`已加载存档 ${slot + 1}`);
            setView("gameplay");
            closeModal("modalLoadGame");
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

async function checkServices() {
    try {
        const result = await window.gameApi.checkServices();
        const alg = result.algorithm.ok ? "Algorithm OK" : `Algorithm ERR: ${result.algorithm.error}`;
        const stg = result.strategy.ok ? "Strategy OK" : `Strategy ERR: ${result.strategy.error}`;
        refs.serviceState.textContent = `${alg} | ${stg}`;
    } catch (error) {
        refs.serviceState.textContent = `Service check failed: ${error.message}`;
    }
}

function bindEvents() {
    refs.btnNewGame.addEventListener("click", () => openModal("modalNewGame"));
    refs.btnLoadGame.addEventListener("click", () => openModal("modalLoadGame"));
    refs.btnSettings.addEventListener("click", () => openModal("modalSettings"));
    refs.btnExit.addEventListener("click", () => window.close());

    refs.btnInventory.addEventListener("click", () => refs.inventoryDrawer.classList.add("open"));
    refs.closeInventory.addEventListener("click", () => refs.inventoryDrawer.classList.remove("open"));

    refs.btnQuest.addEventListener("click", () => showToast("任务系统开发中"));

    refs.btnSkillTree.addEventListener("click", async () => {
        try {
            const response = await window.gameApi.getSkillTree({
                character_id: 1,
                learned_skills: [1, 2]
            });
            showToast(`技能节点: ${response.skills.length}`);
        } catch (error) {
            showToast(`技能树请求失败: ${error.details || error.message}`);
        }
    });

    refs.btnSave.addEventListener("click", async () => {
        try {
            const worldState = {
                chapter: state.world.chapter,
                location: state.world.location,
                day: state.world.day,
                player: state.player,
                position: { x: state.world.x, y: state.world.y }
            };

            await window.gameApi.updateWorldState({
                world_state_json: JSON.stringify(worldState)
            });
            showToast("保存成功");
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
    });

    refs.saveSettings.addEventListener("click", () => {
        closeModal("modalSettings");
        showToast("设置已保存");
    });

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
    buildWorldTiles();
    buildInventory();
    buildSaveSlots();
    bindEvents();
    updateHUD();
    checkServices();
}

bootstrap();
