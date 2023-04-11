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
  var lines_of_code = document.querySelectorAll("#code_area")[0];
  var total_energy_cost = 0;
  
  // TODO: consider adding a BOOP noise
  function confirmationAnimation(elem) {
    var original_color = elem.style.backgroundColor;
    elem.style.backgroundColor = "white";
    setTimeout(function() {
      elem.style.backgroundColor = original_color;
    }, 150);
  }
  
  function collect_code() {
    var current = lines_of_code.firstChild;
    var code_blob = "";
    while (current) {
      code_blob += current.innerHTML + "\n";
      current = current.nextSibling;
    }
    code_blob = code_blob.split(' ').join(',');
    return (code_blob);
  }
  
  // translate the BUCL parameter into an energy cost (SO BRITTLE!)
  var cost_table = { 1: 1, 5: 5, 10: 10, 20: 20, 15: 1, 30: 2, 90: 6, 180: 12 };
  function recalculateCost() {
    var current = lines_of_code.firstChild;
    var sub_total = 0;
    while (current) {
      var text = current.innerHTML;
      sub_total += cost_table[parseInt(text.split(' ')[2])];
      current = current.nextSibling;
    }
    var span = document.querySelector('#total_energy_cost');
    span.innerHTML = "Energy Cost: " + (total_energy_cost + sub_total);
  
    return (sub_total);
  }
  
  function addLineOfCode(evt, id, bg_color) {
    if (lines_of_code.childElementCount <= 9) {
      confirmationAnimation(evt.target);
      var elem = document.createElement("pre");
      // translate the button id into code
      elem.innerHTML = id.split('_').join(' ');
      elem.style.backgroundColor = bg_color;
      lines_of_code.appendChild(elem);
      recalculateCost();
    } else {
      console.log("line limit");
      console.log(lines_of_code.length);
    }
  }
  
  function deleteTheCode() {
    while (lines_of_code.firstChild) {
      lines_of_code.removeChild(lines_of_code.firstChild);
    }
    recalculateCost();
  }
  function clearCode(evt) {
    confirmationAnimation(evt.target);
    deleteTheCode();
    recalculateCost();
  }
  
  function undoCode(evt) {
    confirmationAnimation(evt.target);
    if (lines_of_code.lastChild) {
      lines_of_code.removeChild(lines_of_code.lastChild);
    }
    recalculateCost();
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
    total_energy_cost += recalculateCost();
    var code_blob = collect_code();
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
  
  