#include <QFile>
#include <QString>
#include <QTextStream>

#include "script/scriptengine.h"
#include "script/scriptinterface.h"

ScriptEngine::ScriptEngine(Simulator& sim, VisItem* vis)
    : scriptInterface(new ScriptInterface(*this, sim, vis))
{
    // Create a global object for the javascript engine and make its methods globally accessible.
    // The ownership of ScriptInterface-object goes to the engine.
    auto globalObject = engine.newQObject(scriptInterface);
    engine.globalObject().setProperty("globalObject", globalObject);
    engine.evaluate("Object.keys(globalObject).forEach(function(key){ this[key] = globalObject[key] })");
}

void ScriptEngine::executeCommand(const QString cmd)
{
    auto result = engine.evaluate(cmd);
    if(!result.isUndefined()) {
        emit log(result.toString(), result.isError());
    }
}

void ScriptEngine::runScript(const QString scriptFilePath)
{
    QFile scriptFile(scriptFilePath);

    if(!scriptFile.open(QFile::ReadOnly)) {
        emit log("could not open script file", true);
        return;
    }

    QTextStream stream(&scriptFile);
    const QString script = stream.readAll();

    scriptFile.close();

    engine.evaluate(script);
}
