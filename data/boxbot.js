// Desc: This file contains:
//  - the toolbox for the Blockly editor
//  - the code generator for the Blockly editor
//  - the initialization of the Blockly editor

// TODO: Scott - luminosity block

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
}
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
}

async function fetchWait(url) {
  console.log("fetching: " + url);
  await fetch(url);
  var busy = true;
  while (busy) {
    console.log("busy is " + busy + " " + url);
    var response = await fetch('/busy');
    if (!response.ok) {
      console.log("error: " + response.status);
      busy = false;
    }
    var status = await response.json();
    console.log(status)
    console.log("waiting got: " + response.status);
    busy = status.busy;
  }
}

function asyncWrap(url) {
  return "await fetchWait(" + url + ");\n";
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
  // angle = eval(angle);
  var code = asyncWrap('"/turn?angle=" + Math.round(' + angle + ')');
  return code;
};
Blockly.JavaScript['boxbot_left'] = function(block) {
  var angle = Blockly.JavaScript.valueToCode(block, 'ANGLE', Blockly.JavaScript.ORDER_ATOMIC);
  // angle = eval(angle);
  var code = asyncWrap('"/turn?angle=" + Math.round(-' + angle + ')');
  return code;
};

// initialize Blockly
const workspace = Blockly.inject('blocklyDiv', { toolbox: toolbox });

function generateCode() {
  return Blockly.JavaScript.workspaceToCode(workspace);
}

function runCode() {
  var code = '(async () => {' + generateCode() + '})()';
  console.log("----------");
  console.log(code);
  // TODO: Scott - how do we prevent double-clicking the run button?
  // TODO: Scott - how do abort a running program?
  eval(code);
}