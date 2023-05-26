// TODO: Scott - luminosity block

/**
 * BOXBOT BLOCK DEFINITIONS
 */

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

/**
 * LISP CODE GENERATOR
 */

const Lisp = new Blockly.Generator('Lisp');

// no precedence levels for now, assume we wrap everything in parens
Lisp.PRECEDENCE = 0;

// this joins together code for blocks in a stack
Lisp.scrub_ = function(block, code, opt_thisOnly) {
  const nextBlock = block.nextConnection && block.nextConnection.targetBlock();
  return code + (opt_thisOnly ? '' : this.blockToCode(nextBlock));
};

/**
 * COMMON BLOCK IMPLEMENTATIONS
 */

// This should work, but it seems like there is a bug in `cond` in this LISP implementation!?
/*
Lisp['controls_if'] = function(block) {
  let code = '(cond ';

  let n = 0;
  do {
    const condCode = Lisp.valueToCode(block, 'IF'+n, Lisp.PRECEDENCE) || '()';
    const branchCode = Lisp.statementToCode(block, 'DO'+n) || '()';
    code += `(${condCode} ${branchCode})`;
    n++;
  } while (block.getInput('IF'+n));

  if (block.getInput('ELSE')) {
    const branchCode = Lisp.statementToCode(block, 'ELSE');
    code += `(#t ${branchCode})`;
  }

  code += ')';

  return code;
};
*/

// We have to do nested if instead of cond for now
Lisp['controls_if'] = function(block) {
  let pairs = []; // pairs of (conditionCode, branchCode)

  let n = 0;
  do {
    const condCode = Lisp.valueToCode(block, 'IF'+n, Lisp.PRECEDENCE) || '()';
    const branchCode = Lisp.statementToCode(block, 'DO'+n) || '()';
    pairs.push([condCode, branchCode]);
    n++;
  } while (block.getInput('IF'+n));

  if (block.getInput('ELSE')) {
    const branchCode = Lisp.statementToCode(block, 'ELSE');
    pairs.push(['#t', branchCode]);
  }

  console.log(pairs);

  if (pairs.length === 0) {
    throw new Error('unexpected');
  }

  // iterate in reverse order, building up nested if statements
  let i = pairs.length - 1;
  let code = `(if ${pairs[i][0]} ${pairs[i][1]} ())`;
  i--;
  while (i >= 0) {
    code = `(if ${pairs[i][0]} ${pairs[i][1]} ${code})`;
    i--;
  }

  return code;
};

Lisp['logic_compare'] = function(block) {
  const argACode = Lisp.valueToCode(block, 'A', Lisp.PRECEDENCE) || '0';
  const argBCode = Lisp.valueToCode(block, 'B', Lisp.PRECEDENCE) || '0';

  switch (block.getFieldValue('OP')) {
    case 'EQ':
      return [`(eq? ${argACode} ${argBCode})`, Lisp.PRECEDENCE];

    case 'NEQ':
      return [`(not (eq? ${argACode} ${argBCode}))`, Lisp.PRECEDENCE];

    case 'LT':
      return [`(< ${argACode} ${argBCode})`, Lisp.PRECEDENCE];

    case 'LTE':
      return [`(<= ${argACode} ${argBCode})`, Lisp.PRECEDENCE];

    case 'GT':
      return [`(> ${argACode} ${argBCode})`, Lisp.PRECEDENCE];

    case 'GTE':
      return [`(>= ${argACode} ${argBCode})`, Lisp.PRECEDENCE];

    default:
      throw new Error('unknown op');
  }
};

Lisp['logic_operation'] = function(block) {
  const op = (block.getFieldValue('OP') === 'AND') ? 'and' : 'or';

  // missing args are considered false
  let argACode = Lisp.valueToCode(block, 'A', Lisp.PRECEDENCE);
  let argBCode = Lisp.valueToCode(block, 'B', Lisp.PRECEDENCE);

  if (!argACode && !argBCode) {
    // If there are no arguments, then the return value is false
    argACode = '()';
    argBCode = '()';
  } else {
    // Single missing arguments have no effect on the return value
    const defaultArgCode = (op === 'and') ? '#t' : '()';
    if (!argACode) {
      argACode = defaultArgCode;
    }
    if (!argBCode) {
      argBCode = defaultArgCode;
    }
  }

  const code = `(${op} ${argACode} ${argBCode})`;
  return [code, Lisp.PRECEDENCE];
};

Lisp['logic_negate'] = function(block) {
  const argCode = Lisp.valueToCode(block, 'BOOL', Lisp.PRECEDENCE) || '#t';
  const code = `(not ${argCode})`;
  return [code, Lisp.PRECEDENCE];
};

Lisp['logic_boolean'] = function(block) {
  const code = (block.getFieldValue('BOOL') === 'TRUE') ? '#t' : '()';
  return [code, Lisp.PRECEDENCE];
};

Lisp['logic_ternary'] = function(block) {
  const ifCode = Lisp.valueToCode(block, 'IF', Lisp.PRECEDENCE) || '()';
  const thenCode = Lisp.valueToCode(block, 'THEN', Lisp.PRECEDENCE) || '()';
  const elseCode = Lisp.valueToCode(block, 'ELSE', Lisp.PRECEDENCE) || '()';

  const code = `(if ${ifCode} ${thenCode} ${elseCode})`;
  return [code, Lisp.PRECEDENCE];
};

Lisp['controls_repeat_ext'] = function(block) {
  const repeatsCode = Lisp.valueToCode(block, 'TIMES', Lisp.PRECEDENCE) || '0';
  const innerCode = Lisp.statementToCode(block, 'DO');
  // TODO: add loop trap?

  // because we make a local loop var, we don't need a distinct name
  // TODO: could define dotimes macro and use that, to clarify code
  const code = `(let (i 0) (while (< i ${repeatsCode}) ${innerCode} (setq i (+ i 1))))`;

  return code;
};

Lisp['math_number'] = function(block) {
  var code = String(block.getFieldValue('NUM'));
  return [code, Lisp.PRECEDENCE];
};

/**
 * BOXBOT BLOCK IMPLEMENTATIONS
 */

Lisp['boxbot_forward'] = function(block) {
  const distance = Lisp.valueToCode(block, 'DISTANCE', Lisp.PRECEDENCE);
  return `(bb_forward ${distance})`;
};

Lisp['boxbot_backward'] = function(block) {
  const distance = Lisp.valueToCode(block, 'DISTANCE', Lisp.PRECEDENCE);
  return `(bb_backward ${distance})`;
};

Lisp['boxbot_right'] = function(block) {
  const angle = Lisp.valueToCode(block, 'ANGLE', Lisp.PRECEDENCE);
  return `(bb_right ${angle})`;
};

Lisp['boxbot_left'] = function(block) {
  const angle = Lisp.valueToCode(block, 'ANGLE', Lisp.PRECEDENCE);
  return `(bb_left ${angle})`;
};

// initialize Blockly
const workspace = Blockly.inject('blocklyDiv', {toolbox: toolbox});

function generateCode() {
  return Lisp.workspaceToCode(workspace);
}

function runCode() {
  const code = generateCode()
  console.log("---------- begin generated LISP code ----------");
  console.log(code);
  console.log("---------- end generated LISP code ----------");
  // TODO: Scott - how do we prevent double-clicking the run button?
  // TODO: Scott - how do abort a running program?
}
