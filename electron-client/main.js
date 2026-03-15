const path = require("path");
const { app, BrowserWindow, ipcMain } = require("electron");
const grpc = require("@grpc/grpc-js");
const protoLoader = require("@grpc/proto-loader");

const protoPaths = [
    path.join(__dirname, "..", "proto", "AlgorithmService.proto"),
    path.join(__dirname, "..", "proto", "StrategyService.proto")
];

const packageDefinition = protoLoader.loadSync(protoPaths, {
    keepCase: true,
    longs: String,
    enums: String,
    defaults: true,
    oneofs: true
});

const grpcObject = grpc.loadPackageDefinition(packageDefinition);
const algorithmProto = grpcObject.algorithm_proto;
const strategyProto = grpcObject.strategy_proto;

const algorithmClient = new algorithmProto.AlgorithmService(
    "127.0.0.1:50051",
    grpc.credentials.createInsecure()
);

const strategyClient = new strategyProto.StrategyService(
    "127.0.0.1:50052",
    grpc.credentials.createInsecure()
);

const isDev = process.argv.includes("--dev");

function invokeRpc(client, method, request) {
    return new Promise((resolve, reject) => {
        client[method](request, (error, response) => {
            if (error) {
                reject({
                    code: error.code,
                    details: error.details,
                    message: error.message
                });
                return;
            }
            resolve(response);
        });
    });
}

function createWindow() {
    const window = new BrowserWindow({
        width: 1366,
        height: 820,
        minWidth: 1080,
        minHeight: 680,
        backgroundColor: "#0d1618",
        show: false,
        webPreferences: {
            preload: path.join(__dirname, "preload.js"),
            contextIsolation: true,
            nodeIntegration: false,
            sandbox: true
        }
    });

    window.once("ready-to-show", () => {
        window.show();
        if (isDev) {
            window.webContents.openDevTools({ mode: "detach" });
        }
    });

    window.loadFile(path.join(__dirname, "src", "index.html"));
}

ipcMain.handle("service:health", async () => {
    const result = {
        algorithm: { ok: false, error: null },
        strategy: { ok: false, error: null }
    };

    try {
        await invokeRpc(algorithmClient, "ValidateInput", {
            fields: {},
            validation_type: "health_check"
        });
        result.algorithm.ok = true;
    } catch (error) {
        result.algorithm.error = error.details || error.message;
    }

    try {
        await invokeRpc(strategyClient, "QueryGameState", {
            query_type: "health_check",
            entity_id: 0
        });
        result.strategy.ok = true;
    } catch (error) {
        result.strategy.error = error.details || error.message;
    }

    return result;
});

ipcMain.handle("algorithm:calculate-damage", (_, request) =>
    invokeRpc(algorithmClient, "CalculateDamage", request)
);

ipcMain.handle("algorithm:skill-tree", (_, request) =>
    invokeRpc(algorithmClient, "GetSkillTree", request)
);

ipcMain.handle("algorithm:ai-decision", (_, request) =>
    invokeRpc(algorithmClient, "AIActionDecision", request)
);

ipcMain.handle("strategy:game-rules", (_, request) =>
    invokeRpc(strategyClient, "GetGameRules", request)
);

ipcMain.handle("strategy:update-world", (_, request) =>
    invokeRpc(strategyClient, "UpdateWorldState", request)
);

ipcMain.handle("strategy:trigger-event", (_, request) =>
    invokeRpc(strategyClient, "TriggerEvent", request)
);

ipcMain.handle("strategy:query-state", (_, request) =>
    invokeRpc(strategyClient, "QueryGameState", request)
);

app.whenReady().then(() => {
    createWindow();

    app.on("activate", () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow();
        }
    });
});

app.on("window-all-closed", () => {
    if (process.platform !== "darwin") {
        app.quit();
    }
});
