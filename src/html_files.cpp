/**********************************************************
DO NOT EDIT - BUILT BY THE BUILD SCRIPT: https://replit.com/@gever/bbdebuggui
*************************************************************/ 
const char *script_js= R"VERBATIM(
function report_error(error_message) {
  document.getElementById("error_panel").innerHTML += error_message;
}

// immediate motion 
function boxbot_move(v) {
  fetch('/move?a=' + v); // .then((response) => console.log(response.json()));
}
function boxbot_turn(v) {
  fetch('/turn?a=' + v); // .then((response) => console.log(response.json()));
}
function boxbot_stop(v) {
  fetch('/stop'); // .then((response) => console.log(response.json()));
}
function boxbot_send_plan(plan) {
  fetch('/plan?a=' + plan); //.then((response) => console.log(response.json()));
  console.log('plan: ' + plan);
}

// motion planning support
// add click handlers to all of the command panel buttons
const buttons = document.querySelectorAll('.panel_button');
const lines_of_code = document.querySelectorAll("#lines_of_code")[0];

// TODO: consider adding a BOOP noise
function confirmationAnimation(elem) {
  var original_color = elem.style.backgroundColor;
  elem.style.backgroundColor = "white";
  setTimeout(function() {
    elem.style.backgroundColor = original_color;
  }, 150);
}

function addLineOfCode(evt, id, bg_color) {
  if (lines_of_code.childElementCount <= 9) {
    confirmationAnimation(evt.target);
    var elem = document.createElement("pre");
    // translate the button id into code
    elem.innerHTML = id.split('_').join(' ');
    elem.style.backgroundColor = bg_color;
    lines_of_code.appendChild(elem);
  } else {
    console.log("line limit");
    console.log(lines_of_code.length);
  }
}

function deleteTheCode() {
  while (lines_of_code.firstChild) {
    lines_of_code.removeChild(lines_of_code.firstChild);
  }
}
function clearCode(evt) {
  confirmationAnimation(evt.target);
  deleteTheCode();
}

function undoCode(evt) {
  confirmationAnimation(evt.target);
  if (lines_of_code.lastChild) {
    lines_of_code.removeChild(lines_of_code.lastChild);
  }
}

// encode the "motion plan" in a dense format that is
// easier to parse on the boxbot (until python/lisp are working)
// input:
//   MOV,FWD,10
//   MOV,BWD,10
//   TRN,LT,90
//   TRN,RT,90
//   TRN,LT,30
//   TRN,RT,30
//   MOV,FWD,5
//   MOV,BWD,5
//   TRN,LT,180
//   TRN,RT,180
//  output:
//   M10,M-10,T-90,T90,T-30,T30,M-5,M5,T-180,T180
function boxbot_translate(raw_code) {
  var code = raw_code.split("\n");
  var output = "";
  for (var i = 0; i < code.length; i++) {
    var line = code[i];
    if (line.length == 0) continue;

    var parts = line.split(",");
    var opcode = parts[0];
    var modifier = parts[1];
    var param = parts[2];
    if (output.length > 0) {
      output += ",";
    }
    if (opcode == "TRN") {
      output += "T";
    } else if (opcode == "MOV") {
      output += "M";
    } else {
      return "ERROR: unknown opcode: " + opcode;
    }
    if ((modifier == "LT") || (modifier == "BWD")) {
      output += "-";
    }
    output += param;
  }
  return output;
}

var last_send = Date.now();
function sendCode(evt) {
  if ((Date.now() - last_send) < 1000) {
    // don't let them pound the send button
    return;
  }
  last_send = Date.now();

  // collect the code
  confirmationAnimation(evt.target);
  var current = lines_of_code.firstChild;
  var code_blob = "";
  while (current) {
    code_blob += current.innerHTML + "\n";
    current = current.nextSibling;
  }
  code_blob = code_blob.split(' ').join(',');
  var translated = boxbot_translate(code_blob);
  if (translated.startsWith("ERROR")) {
    console.log(translated);
  } else {
    // report_error("sending: " + translated + "\n")
    boxbot_send_plan(translated);
    // console.log("Code---");
    // console.log(code_blob);
    // window.BU_send( code_blob );
    setTimeout(deleteTheCode, 500);
  }
}

function forceRefresh() {
  window.location.reload();
}

// add the click handlers to the buttons
buttons.forEach(box => {
  box.addEventListener('click', function handleClick(event) {
    addLineOfCode(event, box.id, window.getComputedStyle(box).backgroundColor);
  });
});


)VERBATIM";

const char *blockly_js= R"VERBATIM(
const toolbox1 = {
  "kind": "flyoutToolbox",
  "contents": [
    {
      "kind": "block",
      "type": "controls_if"
    },
    {
      "kind": "block",
      "type": "controls_repeat_ext"
    },
    {
      "kind": "block",
      "type": "logic_compare"
    },
    {
      "kind": "block",
      "type": "math_number"
    },
    {
      "kind": "block",
      "type": "math_arithmetic"
    },
    {
      "kind": "block",
      "type": "text"
    },
    {
      "kind": "block",
      "type": "text_print"
    },
  ]
}
var toolbox = {
  "kind": "categoryToolbox",
  "contents": [
    {
      "kind": "category",
      "name": "Logic",
      "categorystyle": "logic_category",
      "contents": [
        {
          "kind": "category",
          "name": "If",
          "contents": [
            {
              "kind": "block",
              "type": "controls_if"
            },
            {
              "kind": "block",
              "type": "controls_if",
              "extraState": {
                "hasElse": "true"
              }
            },
            {
              "kind": "block",
              "type": "controls_if",
              "extraState": {
                "hasElse": "true",
                "elseIfCount": 1
              }
            }
          ]
        },
        {
          "kind": "category",
          "name": "Boolean",
          "categorystyle": "logic_category",
          "contents": [
            {
              "kind": "block",
              "type": "logic_compare"
            },
            {
              "kind": "block",
              "type": "logic_operation"
            },
            {
              "kind": "block",
              "type": "logic_negate"
            },
            {
              "kind": "block",
              "type": "logic_boolean"
            },
            {
              "kind": "block",
              "type": "logic_null"
            },
            {
              "kind": "block",
              "type": "logic_ternary"
            }
          ]
        }
      ]
    },
    {
      "kind": "category",
      "name": "Loops",
      "categorystyle": "loop_category",
      "contents": [
        {
          "kind": "block",
          "type": "controls_repeat_ext",
          "inputs": {
            "TIMES": {
              "block": {
                "type": "math_number",
                "fields": {
                  "NUM": 10
                }
              }
            }
          }
        },
        {
          "kind": "block",
          "type": "controls_whileUntil"
        },
        {
          "kind": "block",
          "type": "controls_for",
          "fields": {
            "VAR": "i"
          },
          "inputs": {
            "FROM": {
              "block": {
                "type": "math_number",
                "fields": {
                  "NUM": 1
                }
              }
            },
            "TO": {
              "block": {
                "type": "math_number",
                "fields": {
                  "NUM": 10
                }
              }
            },
            "BY": {
              "block": {
                "type": "math_number",
                "fields": {
                  "NUM": 1
                }
              }
            }
          }
        },
        {
          "kind": "block",
          "type": "controls_forEach"
        },
        {
          "kind": "block",
          "type": "controls_flow_statements"
        }
      ]
    },
    {
      "kind": "category",
      "name": "Math",
      "categorystyle": "math_category",
      "contents": [
        {
          "kind": "block",
          "type": "math_number",
          "fields": {
            "NUM": 123
          }
        },
        {
          "kind": "block",
          "type": "math_arithmetic",
          "fields": {
            "OP": "ADD"
          }
        },
        {
          "kind": "block",
          "type": "math_single",
          "fields": {
            "OP": "ROOT"
          }
        },
        {
          "kind": "block",
          "type": "math_trig",
          "fields": {
            "OP": "SIN"
          }
        },
        {
          "kind": "block",
          "type": "math_constant",
          "fields": {
            "CONSTANT": "PI"
          }
        },
        {
          "kind": "block",
          "type": "math_number_property",
          "extraState": "<mutation divisor_input=\"false\"></mutation>",
          "fields": {
            "PROPERTY": "EVEN"
          }
        },
        {
          "kind": "block",
          "type": "math_round",
          "fields": {
            "OP": "ROUND"
          }
        },
        {
          "kind": "block",
          "type": "math_on_list",
          "extraState": "<mutation op=\"SUM\"></mutation>",
          "fields": {
            "OP": "SUM"
          }
        },
        {
          "kind": "block",
          "type": "math_modulo"
        },
        {
          "kind": "block",
          "type": "math_constrain",
          "inputs": {
            "LOW": {
              "block": {
                "type": "math_number",
                "fields": {
                  "NUM": 1
                }
              }
            },
            "HIGH": {
              "block": {
                "type": "math_number",
                "fields": {
                  "NUM": 100
                }
              }
            }
          }
        },
        {
          "kind": "block",
          "type": "math_random_int",
          "kind": "block",
          "inputs": {
            "FROM": {
              "block": {
                "type": "math_number",
                "fields": {
                  "NUM": 1
                }
              }
            },
            "TO": {
              "block": {
                "type": "math_number",
                "fields": {
                  "NUM": 100
                }
              }
            }
          }
        },
        {
          "kind": "block",
          "type": "math_random_float"
        },
        {
          "kind": "block",
          "type": "math_atan2"
        }
      ]
    },
    {
      "kind": "category",
      "name": "Lists",
      "categorystyle": "list_category",
      "contents": [
        {
          "kind": "block",
          "type": "lists_create_empty"
        },
        {
          "kind": "block",
          "type": "lists_create_with",
          "extraState": {
            "itemCount": 3
          }
        },
        {
          "kind": "block",
          "type": "lists_repeat",
          "inputs": {
            "NUM": {
              "block": {
                "type": "math_number",
                "fields": {
                  "NUM": 5
                }
              }
            }
          }
        },
        {
          "kind": "block",
          "type": "lists_length"
        },
        {
          "kind": "block",
          "type": "lists_isEmpty"
        },
        {
          "kind": "block",
          "type": "lists_indexOf",
          "fields": {
            "END": "FIRST"
          }
        },
        {
          "kind": "block",
          "type": "lists_getIndex",
          "fields": {
            "MODE": "GET",
            "WHERE": "FROM_START"
          }
        },
        {
          "kind": "block",
          "type": "lists_setIndex",
          "fields": {
            "MODE": "SET",
            "WHERE": "FROM_START"
          }
        }
      ]
    },
    {
      "kind": "sep"
    },
    {
      "kind": "category",
      "name": "Variables",
      "categorystyle": "variable_category",
      "custom": "VARIABLE"
    },
    {
      "kind": "category",
      "name": "Functions",
      "categorystyle": "procedure_category",
      "custom": "PROCEDURE"
    },
    {
      "kind": "category",
      "name": "Library",
      "expanded": "true",
      "contents": [
        {
          "kind": "category",
          "name": "Randomize",
          "contents": [
            {
              "kind": "block",
              "type": "procedures_defnoreturn",
              "extraState": {
                "params": [
                  {
                    "name": "list"
                  }
                ]
              },
              "icons": {
                "comment": {
                  "text": "Describe this function...",
                  "pinned": false,
                  "height": 80,
                  "width": 160
                }
              },
              "fields": {
                "NAME": "randomize"
              },
              "inputs": {
                "STACK": {
                  "block": {
                    "type": "controls_for",
                    "fields": {
                      "VAR": {
                        "name": "x"
                      }
                    },
                    "inputs": {
                      "FROM": {
                        "block": {
                          "type": "math_number",
                          "fields": {
                            "NUM": 1
                          }
                        }
                      },
                      "TO": {
                        "block": {
                          "type": "lists_length",
                          "inline": false,
                          "inputs": {
                            "VALUE": {
                              "block": {
                                "type": "variables_get",
                                "fields": {
                                  "VAR": {
                                    "name": "list"
                                  }
                                }
                              }
                            }
                          }
                        }
                      },
                      "BY": {
                        "block": {
                          "type": "math_number",
                          "fields": {
                            "NUM": 1
                          }
                        }
                      },
                      "DO": {
                        "block": {
                          "type": "variables_set",
                          "inline": false,
                          "fields": {
                            "VAR": {
                              "name": "y"
                            }
                          },
                          "inputs": {
                            "VALUE": {
                              "block": {
                                "type": "math_random_int",
                                "inputs": {
                                  "FROM": {
                                    "block": {
                                      "type": "math_number",
                                      "fields": {
                                        "NUM": 1
                                      }
                                    }
                                  },
                                  "TO": {
                                    "block": {
                                      "type": "lists_length",
                                      "inline": false,
                                      "inputs": {
                                        "VALUE": {
                                          "block": {
                                            "type": "variables_get",
                                            "fields": {
                                              "VAR": {
                                                "name": "list"
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          },
                          "next": {
                            "block": {
                              "type": "variables_set",
                              "inline": false,
                              "fields": {
                                "VAR": {
                                  "name": "temp"
                                }
                              },
                              "inputs": {
                                "VALUE": {
                                  "block": {
                                    "type": "lists_getIndex",
                                    "fields": {
                                      "MODE": "GET",
                                      "WHERE": "FROM_START"
                                    },
                                    "inputs": {
                                      "VALUE": {
                                        "block": {
                                          "type": "variables_get",
                                          "fields": {
                                            "VAR": {
                                              "name": "list"
                                            }
                                          }
                                        }
                                      },
                                      "AT": {
                                        "block": {
                                          "type": "variables_get",
                                          "fields": {
                                            "VAR": {
                                              "name": "y"
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              },
                              "next": {
                                "block": {
                                  "type": "lists_setIndex",
                                  "inline": false,
                                  "fields": {
                                    "MODE": "SET",
                                    "WHERE": "FROM_START"
                                  },
                                  "inputs": {
                                    "LIST": {
                                      "block": {
                                        "type": "variables_get",
                                        "fields": {
                                          "VAR": {
                                            "name": "list"
                                          }
                                        }
                                      }
                                    },
                                    "AT": {
                                      "block": {
                                        "type": "variables_get",
                                        "fields": {
                                          "VAR": {
                                            "name": "y"
                                          }
                                        }
                                      }
                                    },
                                    "TO": {
                                      "block": {
                                        "type": "lists_getIndex",
                                        "fields": {
                                          "MODE": "GET",
                                          "WHERE": "FROM_START"
                                        },
                                        "inputs": {
                                          "VALUE": {
                                            "block": {
                                              "type": "variables_get",
                                              "fields": {
                                                "VAR": {
                                                  "name": "list"
                                                }
                                              }
                                            }
                                          },
                                          "AT": {
                                            "block": {
                                              "type": "variables_get",
                                              "fields": {
                                                "VAR": {
                                                  "name": "x"
                                                }
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  },
                                  "next": {
                                    "block": {
                                      "type": "lists_setIndex",
                                      "inline": false,
                                      "fields": {
                                        "MODE": "SET",
                                        "WHERE": "FROM_START"
                                      },
                                      "inputs": {
                                        "LIST": {
                                          "block": {
                                            "type": "variables_get",
                                            "fields": {
                                              "VAR": {
                                                "name": "list"
                                              }
                                            }
                                          }
                                        },
                                        "AT": {
                                          "block": {
                                            "type": "variables_get",
                                            "fields": {
                                              "VAR": {
                                                "name": "x"
                                              }
                                            }
                                          }
                                        },
                                        "TO": {
                                          "block": {
                                            "type": "variables_get",
                                            "fields": {
                                              "VAR": {
                                                "name": "temp"
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          ]
        },
        {
          "kind": "category",
          "name": "Jabberwocky",
          "contents": [
            {
              "kind": "block",
              "type": "text_print",
              "inputs": {
                "TEXT": {
                  "block": {
                    "type": "text",
                    "fields": {
                      "TEXT": "'Twas brillig, and the slithy toves"
                    }
                  }
                }
              },
              "next": {
                "block": {
                  "type": "text_print",
                  "inputs": {
                    "TEXT": {
                      "block": {
                        "type": "text",
                        "fields": {
                          "TEXT": "  Did gyre and gimble in the wabe:"
                        }
                      }
                    }
                  },
                  "next": {
                    "block": {
                      "type": "text_print",
                      "inputs": {
                        "TEXT": {
                          "block": {
                            "type": "text",
                            "fields": {
                              "TEXT": "All mimsy were the borogroves,"
                            }
                          }
                        }
                      },
                      "next": {
                        "block": {
                          "type": "text_print",
                          "inputs": {
                            "TEXT": {
                              "block": {
                                "type": "text",
                                "fields": {
                                  "TEXT": "  And the mome raths outgrabe."
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            },
            {
              "kind": "block",
              "type": "text_print",
              "inputs": {
                "TEXT": {
                  "block": {
                    "type": "text",
                    "fields": {
                      "TEXT": "\"Beware the Jabberwock, my son!"
                    }
                  }
                }
              },
              "next": {
                "block": {
                  "type": "text_print",
                  "inputs": {
                    "TEXT": {
                      "block": {
                        "type": "text",
                        "fields": {
                          "TEXT": "  The jaws that bite, the claws that catch!"
                        }
                      }
                    }
                  },
                  "next": {
                    "block": {
                      "type": "text_print",
                      "inputs": {
                        "TEXT": {
                          "block": {
                            "type": "text",
                            "fields": {
                              "TEXT": "Beware the Jubjub bird, and shun"
                            }
                          }
                        }
                      },
                      "next": {
                        "block": {
                          "type": "text_print",
                          "inputs": {
                            "TEXT": {
                              "block": {
                                "type": "text",
                                "fields": {
                                  "TEXT": "  The frumious Bandersnatch!\""
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          ]
        }
      ]
    }
  ]
}

const blocklyArea = document.getElementById('blocklyArea');
const blocklyDiv = document.getElementById('blocklyDiv');
const workspace = Blockly.inject(blocklyDiv, { toolbox: toolbox });
// const workspace = Blockly.inject(blocklyDiv);
const onresize = function(e) {
  // Compute the absolute coordinates and dimensions of blocklyArea.
  var element = blocklyArea;
  let x = 0;
  let y = 0;
  do {
    x += element.offsetLeft;
    y += element.offsetTop;
    element = element.offsetParent;
  } while (element);
  // Position blocklyDiv over blocklyArea.
  blocklyDiv.style.left = x + 'px';
  blocklyDiv.style.top = y + 'px';
  blocklyDiv.style.width = blocklyArea.offsetWidth + 'px';
  blocklyDiv.style.height = blocklyArea.offsetHeight + 'px';
  Blockly.svgResize(workspace);
};
window.addEventListener('resize', onresize, false);
onresize();

function updateCode(event) {
  const code = Blockly.JavaScript.workspaceToCode(workspace);
  // console.log(code + "\n");
  document.getElementById('blocklyCodeArea').innerHTML = code;
}
workspace.addChangeListener(updateCode);
)VERBATIM";

const char *style_css= R"VERBATIM(
html, body {
  font-family: sans-serif;
  height: 100%;
  width: 100%;
  background-color: #000000;
}

.banner {
  font-family: sans-serif;
  font-weight: bold;
  color: #28b4f0;
  background-color: #424242;
}

.button_body {
  border-radius: 5px;
  border-style: solid;
  border-width: 2px;
  cursor: pointer;
  padding: 20px;

  -webkit-touch-callout: none;
  -webkit-user-select: none;
  -khtml-user-select: none;
  -moz-user-select: none;
  -ms-user-select: none;
  user-select: none;
}

.arrow {
  font-size: 30px;
  font-weight: bold;
}

.unselectable {
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
}

.formbuilder-group-label {
  font-weight: bold;
  font-size: large;
}
.tooltip-element {
  font-style: italic;
  font-size: small;
}
.form-group {
  margin-bottom: 20px;
}

/* motion planning styles */
body {
  margin: 0px;
  /*
  width: 1024px;
  height: 768px;
  */
}

td {
  vertical-align: top;
}

.secret_link {
  color: orange;
}
.secret_link:hover {
  cursor: pointer;
}
.container {
  width: 100%;
  height: 100%;
}

.command_container {
  width: 800px;
  height: 200px;
}

.code_container {
  width: 400px;
  height: 100%;
}

.panel_button {
  padding: 0px;
  width: 120px;
  height: 80px;
  text-align: center;
  font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
  font-weight: bold;
  font-size: 20px;
  background-color: white;
  border-radius: 10px;
  /* disable text selection */
  -webkit-user-select: none;
  user-select: none;
}

.panel_title {
  font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
  font-weight: bold;
  font-size: 30px;
  text-align: left;
  width: 100%;
  background-color: rgb(175, 196, 223);
}

.code_button {
  padding: 0px;
  height: 70px;
  width: 100px;
  text-align: center;
  font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
  font-weight: bold;
  font-size: 20px;
  background-color: rgb(51, 204, 84);
  border-radius: 10px;
  /* disable text selection */
  -webkit-user-select: none;
  user-select: none;
}

.clear_plan_button {
  background-color: orange;
}

.undo_button {
  background-color: #ff9500;
}

.abort_button {
  background-color: #ff7070;
}

.button_contents {
  padding-top: 28%;
}

.code_button_contents {
  color: white;
  padding-top: 20%;
}

.fwd_style {
  background-color: yellow;
}

.bwd_style {
  background-color: cyan;
}

.right_style {
  background-color: #FF914A;
}

.left_style {
  background-color: #7FA1FF; 
}

.servo_style {
  background-color: white; 
}

.latest_image {
  width: 800px;
  height: 600px;
  /* background-color: rgb(142, 248, 151); */
  /* background-image: url("latest_image.jpg"); */
}
.instructions {
  background-color: purple;
}

#lines_of_code {
  /* height: 485px; */
  height: 250px;
  padding-left: 10px;
}

#coms_status {
}

pre {
  margin: 2px;
  font-size: 20px;
}
)VERBATIM";

const char *touch_go_html= R"VERBATIM(
<!DOCTYPE html>
<html>

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width">
  <title>Boxbot 0.1</title>
  <link href="style.css" rel="stylesheet" type="text/css" />
</head>

<body>
  <table>
    <tr>
      <td class="button_body" onclick="boxbot_move(1)">FWD 1</td>
      <td class="button_body" onclick="boxbot_move(-1)">BWD 1</td>
      <td class="button_body" onclick="boxbot_turn(-15)">LT 15</td>
      <td class="button_body" onclick="boxbot_turn(15)">
        <div class="arrow">&#10227;</div>
        <div>15</div>
      </td>
    </tr>
    <tr>
      <td class="button_body" onclick="boxbot_move(5)">FWD 5</td>
      <td class="button_body" onclick="boxbot_move(-5)">BWD 5</td>
      <td class="button_body" onclick="boxbot_turn(-45)">LT 45</td>
      <td class="button_body" onclick="boxbot_turn(45)">RT 45</td>
    </tr>
    <tr>
      <td class="button_body" onclick="boxbot_move(10)">FWD 10</td>
      <td class="button_body" onclick="boxbot_move(-10)">BWD 10</td>
      <td class="button_body" onclick="boxbot_turn(-90)">LT 90</td>
      <td class="button_body" onclick="boxbot_turn(90)">RT 90</td>
    </tr>
    <tr>
      <td colspan=4 class="button_body" style="text-align:center" onclick="boxbot_stop()">stop</td>
    </tr>
  </table>

  <a href="settings.html">settings</a>
  <script src="script.js"></script>
</body>

</html>
)VERBATIM";

const char *settings_html= R"VERBATIM(
<!DOCTYPE html>
<html>

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width">
  <title>Boxbot Settings</title>
  <link href="style.css" rel="stylesheet" type="text/css" />
</head>

<body>
  <div class="rendered-form">
    <div class="">
      <h1 access="false" id="control-1770517">Boxbot System Settings</h1>
    </div>
    <form action="/save" method="post" class="settings-form">
      <div class="formbuilder-radio-group form-group">
        <div class="formbuilder-group-label">Set Orientation</div>
        <div class="tooltip-element">Determines which direction the boxbot goes when asked to go forward.</div>
        <div class="radio-group">
          <div class="formbuilder-radio">
            <input name="radio-group-1669745857659" access="false" id="radio-group-1669745857659-0"
              value="wheels-in-front" type="radio" checked="checked">
            <label for="radio-group-1669745857659-0">Wheels in Front</label>
          </div>
          <div class="formbuilder-radio">
            <input name="radio-group-1669745857659" access="false" id="radio-group-1669745857659-1"
              value="wheels-in-back" type="radio">
            <label for="radio-group-1669745857659-1">Wheels in Back</label>
          </div>
        </div>
      </div>
      <div class="form-group">
        <div class="formbuilder-group-label">Step Rate</div>
        <div class="tooltip-element">Sets the delay between motor steps. Less than 950 reduces torque.</div>
        <input type="number" class="form-control" name="motor-step-rate" access="false" value="950" min="100"
          max="10000" id="motor-step-rate">
      </div>
      <div class="form-group">
        <div class="formbuilder-group-label">Linear Turn Fudge</div>
        <div class="tooltip-element">This constant is multiplied by the calculated "idealized" turn result
          (which is based on a 80mm diameter and zero-slip surface) to correct under/over turnage. More
          than 1.0 creates more turn, less than 1.0 creates less turn.</div>
        <input type="text" class="form-control" name="linear-turn-fudge" value="1.0" id="linear-turn-fudge">
      </div>
      <div class="form-group">
        <div class="formbuilder-group-label">Linear Motion Fudge</div>
        <div class="tooltip-element">This constant is multiplied by the calculated "idealized" movement result
          (which is based on a 80mm diameter and zero-slip surface) to correct under/over movement. More
          than 1.0 creates more movement, less than 1.0 creates less movement.</div>
        <input type="text" class="form-control" name="linear-motion-fudge" value="1.0" id="linear-motion-fudge">
      </div>
      <div class="formbuilder-button form-group">
        <button type="submit" class="btn-success btn" name="save-button" access="false" style="success"
          id="save-button">Save</button>
      </div>
    </form>
  </div>
  <script src="script.js"></script>
</body>

</html>
)VERBATIM";

const char *blockly_html= R"VERBATIM(
<!DOCTYPE html>
<html>

<head>
  <meta charset="utf-8">
  <title>Boxbot Blockly Beta</title>
  <style>
    html,
    body {
      height: 100%;
      margin: 0;
    }

    body {
      background-color: #fff;
      font-family: "Gill Sans", sans-serif;
      overflow: hidden;
    }

    h1 {
      font-weight: normal;
      font-size: 140%;
    }

    table {
      height: 100%;
      width: 100%;
    }

    #blocklyArea {
      height: 99%;
      background: #fc9;
      text-align: center;
    }
  </style>
  <script src="https://unpkg.com/blockly/blockly.min.js"></script>
  <script src="https://unpkg.com/blockly/blocks_compressed.js"></script>
  <script src="https://unpkg.com/blockly/javascript_compressed.js"></script>
  <script src="https://unpkg.com/blockly/msg/js/en.js"></script>
</head>

<body>
  <table>
    <tr>
      <td colspan="2">
        Blockly Boxbot 0.1
      </td>
    </tr>
    <tr>
      <td id="blocklyArea">
        <div id="blocklyDiv" style="position: absolute"></div>
      </td>
      <td style="background-color: tan;">
        <pre id="blocklyCodeArea"></pre>
      </td>
    </tr>
  </table>
  <script src="blockly.js"></script>
</body>

</html>
)VERBATIM";

const char *index_html= R"VERBATIM(
<!DOCTYPE html>
<html>
<!-- MOTION PLAN GUI -->

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width">
  <title>boxbot</title>
  <link href="style.css" rel="stylesheet" type="text/css" />
</head>

<body style="width:100%;">
  <div class="banner" style="width: 100%;">Tinkering School Boxbot
    <span class="secret_link" onclick="window.location.href = 'blockly.html'">&nbsp;[Beta 0.2]</span>
  </div>
  <table>
  <tr>
    <td class="command_container">
      <table width="100%;">
        <!-- first row -->
        <tr>
          <td id="MOV_FWD_1" class="panel_button fwd_style">
            <div class="button_contents">Forward 1</div>
          </td>
          <td id="MOV_BWD_1" class="panel_button bwd_style">
            <div class="button_contents">Backward 1</div>
          </td>
          <td id="TRN_LT_15" class="panel_button left_style">
            <div class="button_contents">Left 15</div>
          </td>
          <td id="TRN_RT_15" class="panel_button right_style">
            <div class="button_contents">Right 15</div>
          </td>
        </tr>

        <!-- second row -->
        <tr>
          <td id="MOV_FWD_5" class="panel_button fwd_style">
            <div class="button_contents">Forward 5</div>
          </td>
          <td id="MOV_BWD_5" class="panel_button bwd_style">
            <div class="button_contents">Backward 5</div>
          </td>
          <td id="TRN_LT_30" class="panel_button left_style">
            <div class="button_contents">Left 30</div>
          </td>
          <td id="TRN_RT_30" class="panel_button right_style">
            <div class="button_contents">Right 30</div>
          </td>
        </tr>

        <!-- third row -->
        <tr>
          <td id="MOV_FWD_10" class="panel_button fwd_style">
            <div class="button_contents">Forward 10</div>
          </td>
          <td id="MOV_BWD_10" class="panel_button bwd_style">
            <div class="button_contents">Backward 10</div>
          </td>
          <td id="TRN_LT_90" class="panel_button left_style">
            <div class="button_contents">Left 90</div>
          </td>
          <td id="TRN_RT_90" class="panel_button right_style">
            <div class="button_contents">Right 90</div>
          </td>
        </tr>

        <!-- fourth row -->
        <tr>
          <td id="MOV_FWD_20" class="panel_button fwd_style">
            <div class="button_contents">Forward 20</div>
          </td>
          <td id="MOV_BWD_20" class="panel_button bwd_style">
            <div class="button_contents">Backward 20</div>
          </td>
          <td id="TRN_LT_180" class="panel_button left_style">
            <div class="button_contents">Left 180</div>
          </td>
          <td id="TRN_RT_180" class="panel_button right_style">
            <div class="button_contents">Right 180</div>
          </td>
        </tr>

       <!-- fifth row (Sylvia's experimental servo section!)
        <tr>
          <td id="Servo_CW 90" class="panel_button servo_style">
            <div class="button_contents">Servo CW 90</div>
          </td>
          <td id="Servo CW 180" class="panel_button servo_style">
            <div class="button_contents">Servo CW 180</div>
          </td>
          <td id="Servo CCW 90" class="panel_button servo_style">
            <div class="button_contents">Servo CCW 90</div>
          </td>
          <td id="Servo CCW 180" class="panel_button servo_style">
            <div class="button_contents">Servo CCW 180</div>
          </td>
        </tr> 
        --> 

        
      </table>
    </td>

    <td>
      <div class="code_container">
        <div class="main_pane">
          <div class="panel_title" onclick="forceRefresh(event)">Motion Plan</div>
          <div id="lines_of_code"></div>
        </div>
        <table width="100%" style="vertical-align:bottom;">
          <tr>
            <td>
              <div id="button_undo" class="code_button undo_button" onclick="undoCode(event)">
                <div class="code_button_contents">Undo</div>
              </div>
            </td>
            <td>
              <div id="button_clear" class="code_button clear_plan_button" onclick="clearCode(event)">
                <div class="code_button_contents">Clear Plan</div>
              </div>
            </td>
            <td>
              <div id="button_send" class="code_button" onclick="sendCode(event)">
                <div class="code_button_contents">Uplink</div>
              </div>
            </td>
            <td>
              <div id="button_abort" class="code_button abort_button" onclick="boxbot_stop(event)">
                <div class="code_button_contents">ABORT</div>
              </div>
            </td>
          </tr>
        </table>
      </div>
    </td>
  </tr>
  </table>
  <pre style="color:aliceblue" id="error_panel"></pre>
  <script src="script.js"></script>
</body>

</html>
)VERBATIM";
