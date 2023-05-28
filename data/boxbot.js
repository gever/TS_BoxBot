// Desc: This file contains:
//  - the toolbox for the Blockly editor
//  - the code generator for the Blockly editor
//  - the initialization of the Blockly editor

// TODO: Scott - luminosity block

const urlParams = new URLSearchParams(window.location.search);
const simPort = urlParams.get('simport');

Blockly.Blocks['boxbot_forward'] = {
  init: function() {
    // this.appendDummyInput().appendField("Forward");
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
    // this.appendDummyInput().appendField("Forward");
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
    // this.appendDummyInput().appendField("Forward");
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

let running = false;
let stopRequested = false;

const urlPrefix = simPort ? 'http://localhost:' + simPort : '';

async function bbFetchValue(url, key) {
  const fullUrl = urlPrefix + url;
  const response = await fetch(fullUrl);
  if (!response.ok) {
    throw new Error('bbFetchValue HTTP error ' + response.status)
  }
  const jobj = await response.json();
  if (!jobj.hasOwnProperty(key)) {
    throw new Error('bbFetchValue missing key ' + key);
  }
  return jobj[key];
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

async function bbFetchWait(url) {
  const fullUrl = urlPrefix + url;
  const response = await fetch(fullUrl);
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

function asyncWrap(url) {
  return "await bbFetchWait(" + url + ");\n";
}

Blockly.JavaScript['boxbot_forward'] = function(block) {
  var distance = Blockly.JavaScript.valueToCode(block, 'DISTANCE', Blockly.JavaScript.ORDER_ATOMIC);
  var code = asyncWrap('"/move?dist=" + Math.round(' + distance + ')');
  return code;
};

Blockly.JavaScript['boxbot_backward'] = function(block) {
  var distance = Blockly.JavaScript.valueToCode(block, 'DISTANCE', Blockly.JavaScript.ORDER_ATOMIC);
  var code = asyncWrap('"/move?dist=" + Math.round(-' + distance + ')');
  return code;
};

Blockly.JavaScript['boxbot_right'] = function(block) {
  var angle = Blockly.JavaScript.valueToCode(block, 'ANGLE', Blockly.JavaScript.ORDER_ATOMIC);
  var code = asyncWrap('"/turn?angle=" + Math.round(' + angle + ')');
  return code;
};

Blockly.JavaScript['boxbot_left'] = function(block) {
  var angle = Blockly.JavaScript.valueToCode(block, 'ANGLE', Blockly.JavaScript.ORDER_ATOMIC);
  var code = asyncWrap('"/turn?angle=" + Math.round(-' + angle + ')');
  return code;
};

Blockly.JavaScript['boxbot_luminosity1'] = function(block) {
  const code = 'await bbFetchValue("/luminosity1", "luminosity1")';
  return [code, Blockly.JavaScript.ORDER_AWAIT];
};

Blockly.JavaScript['boxbot_luminosity2'] = function(block) {
  const code = 'await bbFetchValue("/luminosity2", "luminosity2")';
  return [code, Blockly.JavaScript.ORDER_AWAIT];
};

Blockly.JavaScript['boxbot_distance'] = function(block) {
  const code = 'await bbFetchValue("/distance", "distance")';
  return [code, Blockly.JavaScript.ORDER_AWAIT];
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
  }
}
running = false;
stopRequested = false;
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
  updateButtons();
  eval(code);
}

function stop() {
  stopRequested = true;
  updateButtons();
}

updateButtons();
document.getElementById('run-button').addEventListener('click', run);
document.getElementById('stop-button').addEventListener('click', stop);
