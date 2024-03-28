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

Blockly.Blocks['boxbot_wait'] = {
  init: function() {
    this.appendValueInput("TIME")
      .setCheck("Number")
      .setAlign(Blockly.ALIGN_RIGHT)
      .appendField("wait");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#fda030");
    this.setTooltip("units are seconds");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['boxbot_set_servo_angle'] = {
  init: function() {
    this.appendValueInput("SERVO_ID")
        .setCheck("Number")
        .appendField("set angle of servo");
    this.appendValueInput("ANGLE")
        .setCheck("Number")
        .appendField("to");
    this.appendDummyInput()
        .appendField("degrees");
    this.setInputsInline(true);
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#e07700");
    this.setTooltip("");
    this.setHelpUrl("");
  }
};

Blockly.Blocks['boxbot_led'] = {
  init: function() {
    this.appendValueInput("PIN")
        .setCheck("Number")
        .appendField("set LED on pin");
    this.appendValueInput("STATE")
        .setCheck("Boolean")
        .appendField("to");
    this.setInputsInline(true);
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#e07700");
    this.setTooltip("");
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

Blockly.Blocks['boxbot_log_commands'] = {
  init: function() {
    this.appendValueInput("enable")
        .setCheck("Boolean")
        .appendField("log commands?");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour("#d09000");
    this.setTooltip("should commands be logged?");
    this.setHelpUrl("");
  }
};

let running = false;
let stopRequested = false;

let urlPrefix = '';

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

function logCommandMsg(msg, color) {
  if (logCommands) {
    appendLogMsg(msg, color);
  }
}

function clearLog() {
  const logList = document.getElementById('log-list');
  while (logList.firstChild) {
    logList.removeChild(logList.firstChild);
  }
}

function debugLogWithTime(msg) {
  console.log((Date.now()/1000) + ' ' + msg);
}

async function bbFetchValue(url, key) {
  const fullUrl = urlPrefix + url;
  let response;
  debugLogWithTime('bbFetchValue before fetch ' + fullUrl);
  try {
    response = await fetch(fullUrl);
  } catch (e) {
    throw new Error('can\'t connect to boxbot');
  }
  if (!response.ok) {
    throw new Error('bbFetchValue HTTP error ' + response.status)
  }
  debugLogWithTime('bbFetchValue before json decode ' + fullUrl);
  const jobj = await response.json();
  if (!jobj.hasOwnProperty(key)) {
    throw new Error('bbFetchValue missing key ' + key);
  }
  debugLogWithTime('bbFetchValue returning ' + fullUrl + ' ' + key);
  return jobj[key];
}

async function bbLogFetchValue(url, key) {
  const value = await bbFetchValue(url, key);
  logCommandMsg(key + '? ' + value);
  return value;
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

async function bbFetchWait(url) {
  const fullUrl = urlPrefix + url;
  let response;
  debugLogWithTime('bbFetchWait before initial fetch ' + fullUrl);
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
    debugLogWithTime('bbFetchWait before busy fetch ' + fullUrl);
    const response = await fetch(urlPrefix + '/busy');
    if (!response.ok) {
      throw new Error('bbFetchWait /busy HTTP error ' + response.status)
    }
    debugLogWithTime('bbFetchWait before json decode ' + fullUrl);
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

    debugLogWithTime('bbFetchWait before sleep ' + fullUrl);
    await sleep(50);
  }

  debugLogWithTime('bbFetchWait returning ' + fullUrl);
}

Blockly.JavaScript['boxbot_forward'] = function(block) {
  var distance = Blockly.JavaScript.valueToCode(block, 'DISTANCE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `logCommandMsg('forward ' + Math.round(${distance}));\nawait bbFetchWait("/move?dist=" + Math.round(${distance}));\n`;
  return code;
};

Blockly.JavaScript['boxbot_backward'] = function(block) {
  var distance = Blockly.JavaScript.valueToCode(block, 'DISTANCE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `logCommandMsg('backward ' + Math.round(${distance}));\nawait bbFetchWait("/move?dist=" + Math.round(-${distance}));\n`;
  return code;
};

Blockly.JavaScript['boxbot_right'] = function(block) {
  var angle = Blockly.JavaScript.valueToCode(block, 'ANGLE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `logCommandMsg('right ' + Math.round(${angle}));\nawait bbFetchWait("/turn?angle=" + Math.round(${angle}));\n`;
  return code;
};

Blockly.JavaScript['boxbot_left'] = function(block) {
  var angle = Blockly.JavaScript.valueToCode(block, 'ANGLE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `logCommandMsg('left ' + Math.round(${angle}));\nawait bbFetchWait("/turn?angle=" + Math.round(-${angle}));\n`;
  return code;
};

function wait(time) {
  return new Promise(resolve => setTimeout(resolve, 1000*time));
}

async function interruptableWait(time) {
  const endTime = Date.now() + 1000*time;
  while (true) {
    const time = Date.now();
    if (time >= endTime) {
      break;
    }
    if (stopRequested) {
      throw new AbortError();
    }
    await wait(0.05);
  }
}

Blockly.JavaScript['boxbot_wait'] = function(block) {
  var time = Blockly.JavaScript.valueToCode(block, 'TIME', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `logCommandMsg('waiting ' + ${time} + ' seconds');\nawait interruptableWait(${time});\n`;
  return code;
};

async function bbSetServoAngle(servo_id, angle) {
  if (!Number.isInteger(servo_id) || (servo_id < 0)) {
    return;
  }
  // TODO: generalize the servo initialization to allow for different min/max angles
  const adjAngle = Math.round(angle);
  logCommandMsg('set servo ID ' + servo_id + ' angle ' + adjAngle);
  await bbFetchWait("/servoGo?pin=" + servo_id + "&angle=" + adjAngle);
}
Blockly.JavaScript['boxbot_set_servo_angle'] = function(block) {
  const servo_id = Blockly.JavaScript.valueToCode(block, 'SERVO_ID', Blockly.JavaScript.ORDER_ATOMIC);
  const angleCode = Blockly.JavaScript.valueToCode(block, 'ANGLE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `await bbSetServoAngle(${servo_id}, ${angleCode});\n`;
  return code;
};

async function bbLed(pin, state) {
  if (!Number.isInteger(pin) || (pin < 0)) {
    return;
  }
  logCommandMsg('set LED pin ' + pin + ' state ' + state);
  await bbFetchWait("/led?pin=" + pin + "&state=" + (state ? 1 : 0));
}
Blockly.JavaScript['boxbot_led'] = function(block) {
  const pinCode = Blockly.JavaScript.valueToCode(block, 'PIN', Blockly.JavaScript.ORDER_ATOMIC);
  const stateCode = Blockly.JavaScript.valueToCode(block, 'STATE', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `await bbLed(${pinCode}, ${stateCode});\n`;
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

let logCommands = true;

Blockly.JavaScript['boxbot_log'] = function(block) {
  var text = Blockly.JavaScript.valueToCode(block, 'TEXT', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `appendLogMsg(${text});\n`;
  return code;
};

Blockly.JavaScript['boxbot_log_commands'] = function(block) {
  const enableCode = Blockly.JavaScript.valueToCode(block, 'enable', Blockly.JavaScript.ORDER_ATOMIC);
  const code = `logCommands = ${enableCode};\n`;
  return code;
};

// create custom theme
const theme = Blockly.Theme.defineTheme('boxbotTheme', {
  'base': Blockly.Themes.Classic,
  'blockStyles': {
    'math_blocks': {
      'colourPrimary': '#e75480',
    },
  },
  'categoryStyles': {
    'math_category': {
      'colour': '#e75480',
    },
  },
  'startHats': true
});

// initialize Blockly
const workspace = Blockly.inject('blockly-container', {
  toolbox: toolbox,
  theme: theme,
  media: '/',
});

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
logCommands = true;

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

function genRandom32() {
  return Math.random().toString(16).substring(2, 10);
}
function genRandomUID() {
  return genRandom32() + genRandom32();
}

const OLD_PROGRAM_KEY = 'boxbot-blockly-workspace';
const PROGRAM_INDEX_KEY = 'boxbot-blockly-progidx';
const PROGRAM_KEY_PREFIX = 'boxbot-blockly-program-';
const DEFAULT_PROGRAM_NAME = 'my program';

/*
interface ProgramIndex {
  currentId: string;
  programs: // object with keys as program ids and values as program names
}
*/
let programIndex;

function loadProgram(progId) {
  const progJSON = localStorage.getItem(PROGRAM_KEY_PREFIX + progId);
  prog = JSON.parse(progJSON);
  Blockly.serialization.workspaces.load(prog, workspace);
}

function saveProgramIndex() {
  localStorage.setItem(PROGRAM_INDEX_KEY, JSON.stringify(programIndex));
}

function saveCurrentProgram() {
  const serWS = Blockly.serialization.workspaces.save(workspace);
  localStorage.setItem(PROGRAM_KEY_PREFIX + programIndex.currentId, JSON.stringify(serWS));
}

function updateProgramSelector() {
  const selectElem = document.getElementById('program-selector');
  selectElem.innerHTML = '';
  for (const progId in programIndex.programs) {
    const optionElem = document.createElement('option');
    optionElem.value = progId;
    optionElem.innerText = programIndex.programs[progId];
    selectElem.appendChild(optionElem);
  }
  selectElem.value = programIndex.currentId;
}

// load from local storage
programIndexJSON = localStorage.getItem(PROGRAM_INDEX_KEY);
if (programIndexJSON) {
  programIndex = JSON.parse(programIndexJSON);
  loadProgram(programIndex.currentId);
} else {
  // there was no program index, so check old storage system
  const savedWS = localStorage.getItem(OLD_PROGRAM_KEY);
  if (savedWS) {
    Blockly.serialization.workspaces.load(JSON.parse(savedWS), workspace);
  }

  // initialize the program index
  const progId = genRandomUID();
  programIndex = {
    currentId: progId,
    programs: {
      [progId]: DEFAULT_PROGRAM_NAME,
    },
  };

  saveProgramIndex();
}

saveCurrentProgram();
updateProgramSelector();

document.getElementById('program-selector').addEventListener('change', (event) => {
  const progId = event.target.value;
  programIndex.currentId = progId;
  saveProgramIndex();
  loadProgram(progId);
});

document.getElementById('new-program-button').addEventListener('click', (event) => {
  const progId = genRandomUID();
  programIndex.currentId = progId;
  programIndex.programs[progId] = DEFAULT_PROGRAM_NAME + ' ' + Object.keys(programIndex.programs).length;
  saveProgramIndex();
  updateProgramSelector();
  workspace.clear();
});

document.getElementById('rename-program-button').addEventListener('click', (event) => {
  const newName = prompt('New program name:');
  if (!newName) {
    return;
  }
  programIndex.programs[programIndex.currentId] = newName;
  saveProgramIndex();
  updateProgramSelector();
});

document.getElementById('delete-program-button').addEventListener('click', (event) => {
  if (confirm('Are you sure you want to delete this program?')) {
    delete programIndex.programs[programIndex.currentId];

    // if there are no programs left, create a new one
    if (Object.keys(programIndex.programs).length === 0) {
      const progId = genRandomUID();
      programIndex.currentId = progId;
      programIndex.programs[progId] = DEFAULT_PROGRAM_NAME;
      workspace.clear();
      saveCurrentProgram();
    } else {
      // otherwise, set the current program to the first one
      programIndex.currentId = Object.keys(programIndex.programs)[0];
      loadProgram(programIndex.currentId);
    }
    saveProgramIndex();
    updateProgramSelector();
  }
});

// add a listener to save the workspace to local storage on any change
workspace.addChangeListener((event) => {
  saveCurrentProgram();
});

