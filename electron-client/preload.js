const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("gameApi", {
    checkServices: () => ipcRenderer.invoke("service:health"),
    calculateDamage: (request) => ipcRenderer.invoke("algorithm:calculate-damage", request),
    getSkillTree: (request) => ipcRenderer.invoke("algorithm:skill-tree", request),
    aiDecision: (request) => ipcRenderer.invoke("algorithm:ai-decision", request),
    getGameRules: (request) => ipcRenderer.invoke("strategy:game-rules", request),
    updateWorldState: (request) => ipcRenderer.invoke("strategy:update-world", request),
    triggerEvent: (request) => ipcRenderer.invoke("strategy:trigger-event", request),
    queryGameState: (request) => ipcRenderer.invoke("strategy:query-state", request)
});
