const path = require("path");
const { app, BrowserWindow, ipcMain } = require("electron");
const grpc = require("@grpc/grpc-js");
const protoLoader = require("@grpc/proto-loader");

const ALGORITHM_ADDR = process.env.ALGORITHM_ADDR || "127.0.0.1:50051";
const STRATEGY_ADDR = process.env.STRATEGY_ADDR || "127.0.0.1:50052";
const RPC_TIMEOUT_MS = Number(process.env.RPC_TIMEOUT_MS || 5000);

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
    ALGORITHM_ADDR,
    grpc.credentials.createInsecure()
);

const strategyClient = new strategyProto.StrategyService(
    STRATEGY_ADDR,
    grpc.credentials.createInsecure()
);

const isDev = process.argv.includes("--dev");

function invokeRpc(client, method, request, timeoutMs = RPC_TIMEOUT_MS) {
    return new Promise((resolve, reject) => {
        const metadata = new grpc.Metadata();
        const deadline = new Date(Date.now() + timeoutMs);

        client[method](request, metadata, { deadline }, (error, response) => {
            if (error) {
                reject({
                    code: error.code,
                    details: error.details,
                    message: error.message,
                    method
                });
                return;
            }
            resolve(response);
        });
    });
}

async function checkServiceHealth() {
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
}

async function getBootstrapData() {
    const [health, rules, state] = await Promise.all([
        checkServiceHealth(),
        invokeRpc(strategyClient, "GetGameRules", { category: "combat", active_only: true }).catch(() => null),
        invokeRpc(strategyClient, "QueryGameState", { query_type: "world", entity_id: 0 }).catch(() => null)
    ]);

    return { health, rules, state };
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

ipcMain.handle("service:health", () => checkServiceHealth());
ipcMain.handle("game:bootstrap", () => getBootstrapData());

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
