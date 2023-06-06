const urlParams = new URLSearchParams(window.location.search);
const simPort = urlParams.get('simport');

Blockly.Blocks['boxbot_forward'] = {
  init: function() {
    this.appendValueInput("DISTANCE")
      .setCheck("Number")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("forward");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#fb8500");
    this.setTooltip("units are cm");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['boxbot_backward'] = {
  init: function() {
    this.appendValueInput("DISTANCE")
      .setCheck("Number")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("backward");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#fb8500");
    this.setTooltip("units are cm");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['boxbot_right'] = {
  init: function() {
    this.appendValueInput("ANGLE")
      .setCheck("Number")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("right");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#ffb700");
    this.setTooltip("units are degrees");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['boxbot_left'] = {
  init: function() {
    this.appendValueInput("ANGLE")
      .setCheck("Number")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("left");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#ffb700");
    this.setTooltip("units are degrees");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['boxbot_luminosity1'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("luminosity1");
    this.setOutput(true, "Number");
    this.setColour("#fd6600");
    this.setTooltip("");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['boxbot_luminosity2'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("luminosity2");
    this.setOutput(true, "Number");
    this.setColour("#fd6600");
    this.setTooltip("");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['boxbot_distance'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("distance");
    this.setOutput(true, "Number");
    this.setColour("#fd6600");
    this.setTooltip("");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['boxbot_log'] = {
  init: function() {
    this.appendValueInput("TEXT")
      .setCheck("String")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("log message");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#d09000");
    this.setTooltip("write a message to the log");
    this.setHelpUrl("");
  }
};

let running = false;
let stopRequested = false;

let urlPrefix = '';
if (simPort) {
  const parsedUrl = new URL(window.location.href);
  urlPrefix = parsedUrl.protocol + '//' + parsedUrl.hostname + ':' + simPort;
}

function appendLogMsg(msg, color) {
  const logList = document.getElementById('log-list');
  const li = document.createElement('li');
  li.textContent = msg;
  if (color) {
    li.style.color = color;
  }
  logList.appendChild(li);
  li.scrollIntoView();
}

function clearLog() {
  const logList = document.getElementById('log-list');
  while (logList.firstChild) {
    logList.removeChild(logList.firstChild);
  }
}

async function bbFetchValue(url, key) {
  const fullUrl = urlPrefix + url;
  let response;
  try {
    response = await fetch(fullUrl);
  } catch (e) {
    throw new Error('can\'t connect to boxbot');
  }
  if (!response.ok) {
    throw new Error('bbFetchValue HTTP error ' + response.status)
  }
  const jobj = await response.json();
  if (!jobj.hasOwnProperty(key)) {
    throw new Error('bbFetchValue missing key ' + key);
  }
  return jobj[key];
}

async function bbLogFetchValue(url, key) {
  const value = await bbFetchValue(url, key);
  appendLogMsg(key + '? ' + value);
  return value;
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

async function bbFetchWait(url) {
  const fullUrl = urlPrefix + url;
  let response;
  try {
    response = await fetch(fullUrl);
  } catch (e) {
    throw new Error('can\'t connect to boxbot');
  }
  if (!response.ok) {
    throw new Error('bbFetchWait HTTP error ' + response.status)
  }
  // TODO: check response JSON?

  while (true) {
    const response = await fetch(urlPrefix + '/busy');
    if (!response.ok) {
      throw new Error('bbFetchWait /busy HTTP error ' + response.status)
    }
    const status = await response.json();
    busy = status.busy;
    if (!busy) {
      break;
    }

    if (stopRequested) {
      const response = await fetch(urlPrefix + '/stop');
      if (!response.ok) {
        throw new Error('bbFetchWait /stop HTTP error ' + response.status)
      }
      // don't bother checking response JSON

      throw new AbortError();
    }

    await sleep(50);
  }
}

Blockly.JavaScript['boxbot_forward'] = function(block) {
  var distance = Blockly.JavaScript.valueToCode(block, 'DISTANCE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `appendLogMsg('forward ' + Math.round(${distance}));\nawait bbFetchWait("/move?dist=" + Math.round(${distance}));\n`;
  return code;
};

Blockly.JavaScript['boxbot_backward'] = function(block) {
  var distance = Blockly.JavaScript.valueToCode(block, 'DISTANCE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `appendLogMsg('backward ' + Math.round(${distance}));\nawait bbFetchWait("/move?dist=" + Math.round(-${distance}));\n`;
  return code;
};

Blockly.JavaScript['boxbot_right'] = function(block) {
  var angle = Blockly.JavaScript.valueToCode(block, 'ANGLE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `appendLogMsg('right ' + Math.round(${angle}));\nawait bbFetchWait("/turn?angle=" + Math.round(${angle}));\n`;
  return code;
};

Blockly.JavaScript['boxbot_left'] = function(block) {
  var angle = Blockly.JavaScript.valueToCode(block, 'ANGLE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `appendLogMsg('left ' + Math.round(${angle}));\nawait bbFetchWait("/turn?angle=" + Math.round(-${angle}));\n`;
  return code;
};

Blockly.JavaScript['boxbot_luminosity1'] = function(block) {
  const code = 'await bbLogFetchValue("/luminosity1", "luminosity1")';
  return [code, Blockly.JavaScript.ORDER_AWAIT];
};

Blockly.JavaScript['boxbot_luminosity2'] = function(block) {
  const code = 'await bbLogFetchValue("/luminosity2", "luminosity2")';
  return [code, Blockly.JavaScript.ORDER_AWAIT];
};

Blockly.JavaScript['boxbot_distance'] = function(block) {
  const code = 'await bbLogFetchValue("/distance", "distance")';
  return [code, Blockly.JavaScript.ORDER_AWAIT];
};

Blockly.JavaScript['boxbot_log'] = function(block) {
  var text = Blockly.JavaScript.valueToCode(block, 'TEXT', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `appendLogMsg(${text});\n`;
  return code;
};

// initialize Blockly
const workspace = Blockly.inject('blockly-container', { toolbox: toolbox });

// inject calls to highlight the currently executing block
Blockly.JavaScript.STATEMENT_PREFIX = 'if (stopRequested) { throw new AbortError(); }\nhighlightBlock(%1);\n';
Blockly.JavaScript.addReservedWords('highlightBlock');
function highlightBlock(id) {
  workspace.highlightBlock(id);
}

function AbortError() {
  this.message = 'aborted by user request';
}
AbortError.prototype = new Error;
AbortError.prototype.name = 'AbortError';

function generateCode() {
  const blocklyCode = Blockly.JavaScript.workspaceToCode(workspace);

  const wrappedCode = `
(async () => {
try {

${blocklyCode}
} catch (e) {
  if (e instanceof AbortError) {
    // do nothing
  } else {
    console.log(e);
    appendLogMsg('ERROR: ' + e.message, 'red');
  }
}
running = false;
stopRequested = false;
appendLogMsg('done');
updateButtons();
highlightBlock(null);
})()
`;

  return wrappedCode;
}

function updateButtons() {
  if (running) {
    document.getElementById('run-button').disabled = true;
    document.getElementById('stop-button').disabled = stopRequested;
  } else {
    document.getElementById('run-button').disabled = false;
    document.getElementById('stop-button').disabled = true;
  }
}

function run() {
  if (running) {
    return;
  }

  const code = generateCode();

  console.log(code);

  running = true;
  stopRequested = false;
  clearLog();
  appendLogMsg('running');
  updateButtons();
  eval(code);
}

function stop() {
  stopRequested = true;
  appendLogMsg('stopping');
  updateButtons();
}

updateButtons();
document.getElementById('run-button').addEventListener('click', run);
document.getElementById('stop-button').addEventListener('click', stop);

// add keyboard shortcuts
document.addEventListener('keydown', (event) => {
  // ignore the keypress if it was in an input field
  if (event.target.tagName === 'INPUT') {
    return;
  }

  switch (event.code) {
    case 'KeyR':
      run();
      break;
    case 'KeyS':
      stop();
      break;
  }
});

const STORAGE_KEY = 'boxbot-blockly-workspace';

// load the workspace from local storage
const savedWS = localStorage.getItem(STORAGE_KEY);
if (savedWS) {
  Blockly.serialization.workspaces.load(JSON.parse(savedWS), workspace);
}

// add a listener to save the workspace to local storage on any change
workspace.addChangeListener((event) => {
  const serWS = Blockly.serialization.workspaces.save(workspace);
  localStorage.setItem(STORAGE_KEY, JSON.stringify(serWS));
});

