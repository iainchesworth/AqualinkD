var _width;
var _height;
var _displayNames = [];

function getSize() {
    _width = Math.max(document.documentElement.clientWidth, window.innerWidth || 0);
    _height = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
}

function formatSatus(status) {
    if ((index = status.indexOf("AUX")) >= 0) {
        aux = status.substr(index, 4);
        status = status.charAt(0).toUpperCase() + status.substr(1).toLowerCase();
        if ((name = _displayNames[aux]) != undefined) {
            status = status.substr(0, index) + name + status.substr(index + 4);
        }
    } else {
        status = status.charAt(0).toUpperCase() + status.substr(1).toLowerCase();
    }
    return status;
}

function deviceSort(a, b) {
    try {
        pa = devices.indexOf(a.id);
        pb = devices.indexOf(b.id);
        if (pa > pb)
            return 1;
        else if (pa < pb)
            return -1;
    } catch (e) {}

    return 0;
}

function check_devices(data) {
    _temperature_units = data['temp_units'];
    //document.getElementById("title").innerHTML = "AqualinkD &nbsp;&nbsp;&nbsp;<font size='-2'>"+formatSatus(data['time']+"&nbsp;"+data['date']+"</font>");
    setTitle(data['date'], data['time']);
    data['devices'].sort(deviceSort);
    for (var obj in data['devices']) {
        if (document.getElementById(data['devices'][obj].id) == null) {
            _displayNames[data['devices'][obj].id.replace("Aux_", "AUX")] = data['devices'][obj].name;
            //console.log("Create " + data['devices'][obj].id);
            createTile(data['devices'][obj]);
        }
    }
}

function setTitle(date, time) {
    document.getElementById("title").innerHTML = "AqualinkD &nbsp;&nbsp;&nbsp;<font size='-1'>" + formatSatus(date + "&nbsp;" + time) + "</font>";
}

function update_status(data) {
    //console.log("Received Update Satus");
    _aqualink_data = data;

    if (data.status != "Ready") {
        document.getElementById("title").innerHTML = formatSatus(data.status);
    } else {
        //document.getElementById("title").innerHTML = 'AqualinkD';
        setTitle(data['date'], data['time']);
    }

    setSlider("Pool_Heater", data.pool_htr_set_pnt);
    setSlider("Spa_Heater", data.spa_htr_set_pnt);
    setSlider("SWG", data.swg_percent);
    setSlider("Freeze_Protect", data.frz_protect_set_pnt);
    setLabel("SWG/PPM", data.swg_ppm);
    setLabel("Temperature/Air", data.air_temp);
    setLabel("Temperature/Pool", data.pool_temp);
    setLabel("Temperature/Spa", data.spa_temp);
    /*
    setLabel("Air", data.air_temp);
    setLabel("Pool_Water", data.pool_temp);
    setLabel("Spa_Water", data.spa_temp);
    */

    for (var obj in data.leds) {
        setButton(obj.toString(), data.leds[obj])
    }
}

function createTile(object) {
    if (object.name == 'NONE')
        return;
    //if (ignore_devices.indexOf(object.id) >= 0)
    //  return;
    var table = document.getElementById("deviceList");
    var row = table.insertRow(-1);
    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);
    var cell3 = row.insertCell(2);
    var cell4 = row.insertCell(3);
    cell1.innerHTML = object.name;
    cell2.innerHTML = "&nbsp;";
    cell3.innerHTML = "&nbsp;";
    cell4.innerHTML = "&nbsp;";
    cell2.setAttribute('width', '100px');
    cell3.setAttribute('width', '38px');
    cell4.setAttribute('width', '70px');
    cell3.setAttribute('align', 'right');
    //cell3.setAttribute('cellpadding', '0');
    cell4.setAttribute('align', 'right');
    if (object.type == 'switch') {
        row.deleteCell(2);
        row.deleteCell(1);
        cell1.setAttribute('colspan', '3');
        cell4.innerHTML = "<div id='" + object.id + "'><input id='switch-" + object.id + "' name='" + object.id + "' class='switch switch-btn' type='checkbox' onclick='update(this);'><label for='switch-" + object.id + "'></label></div>";
        setButton(object.id, object.state);
    } else if (object.type == 'switch_program') {
        row.deleteCell(2);
        cell2.setAttribute('colspan', '2');
        //cell2.setAttribute('width', '100%');
        cell2.innerHTML = "<select id='selector_" + object.id + "' onchange='update(this);' class='options_selector'><option value='0'>Program</option></select>";
        cell4.innerHTML = "<div id='" + object.id + "'><input id='switch-" + object.id + "' name='" + object.id + "' class='switch switch-btn' type='checkbox' onclick='update(this);'><label for='switch-" + object.id + "'></label></div>";
        //cell4.innerHTML = "<div class='switch'><input id='btn-toggle-"+object.id+"' name='"+object.id+"' class='btn-toggle btn-toggle-round' type='checkbox' onclick='update(this);'><label for='btn-toggle-"+object.id+"'></label></div>";
        option = document.getElementById('selector_' + object.id);

        try {
            fLen = light_program.length;
            for (i = 0; i < fLen; i++) {
                var opt = document.createElement('option');
                opt.value = i + 1;
                opt.innerHTML = light_program[i];
                option.appendChild(opt);
            }
        } catch (e) {}
        setButton(object.id, object.state);
    } else if (object.type == 'value') {
        row.deleteCell(2);
        row.deleteCell(1);
        cell1.setAttribute('colspan', '3');
        cell4.innerHTML = "<label id='" + object.id + "' output_ext=''></label>";
        setLabel(object.id, object.value);
    } else if (object.type == 'temperature') {
        row.deleteCell(2);
        row.deleteCell(1);
        cell1.setAttribute('colspan', '3');
        cell4.innerHTML = "<label id='" + object.id + "' output_ext='&deg;" + _temperature_units + "'></label>";
        setLabel(object.id, object.value);
    } else if (object.type == 'setpoint_thermo') {
        if (_temperature_units != 'c')
            cell2.innerHTML = "<div class='slidecontainer'><input type='range' id='slider_" + object.id + "' min='36' max='104' class='slider' output_id='" + object.id + "' oninput='sliderchange(this);' onmousedown='lock(this);' onmouseup='update(this);unlock(this);'></div>";
        else
            cell2.innerHTML = "<div class='slidecontainer'><input type='range' id='slider_" + object.id + "' min='0' max='40' class='slider' output_id='" + object.id + "' oninput='sliderchange(this);' onmousedown='lock(this);' onmouseup='update(this);unlock(this);'></div>";

        cell3.innerHTML = "<label id='" + object.id + "' output_ext='&deg;" + _temperature_units + "'></label>";
        cell4.innerHTML = "<div><input id='switch-" + object.id + "' name='" + object.id + "' class='switch switch-btn' type='checkbox' onclick='update(this);'><label for='switch-" + object.id + "'></label></div>";
        //cell4.innerHTML = "<div class='switch'><input id='btn-toggle-"+object.id+"' name='"+object.id+"' class='btn-toggle btn-toggle-round' type='checkbox' onclick='update(this);'><label for='btn-toggle-"+object.id+"'></label></div>";
        setSlider(object.id, object.spvalue);
        setButton(object.id, object.status);
    } else if (object.type == 'setpoint_freeze') {
        if (_temperature_units != 'c')
            cell2.innerHTML = "<div class='slidecontainer'><input type='range' id='slider_" + object.id + "' min='36' max='42' class='slider'  output_id='" + object.id + "' oninput='sliderchange(this);' onmousedown='lock(this);' onmouseup='update(this);unlock(this);'></div>";
        else
            cell2.innerHTML = "<div class='slidecontainer'><input type='range' id='slider_" + object.id + "' min='1' max='5' class='slider'  output_id='" + object.id + "' oninput='sliderchange(this);' onmousedown='lock(this);' onmouseup='update(this);unlock(this);'></div>";

        cell4.innerHTML = "<label id='" + object.id + "' output_ext='&deg;" + _temperature_units + "'></label>";
        setSlider(object.id, object.spvalue);
    } else if (object.type == 'setpoint_swg') {
        if (_width <= 390)
            cell1.innerHTML = "SWG";
        cell2.innerHTML = "<div class='slidecontainer'><input type='range' id='slider_" + object.id + "' min='0' max='100' step='5' class='slider' output_id='" + object.id + "'' oninput='sliderchange(this);' onmousedown='lock(this);' onmouseup='update(this);unlock(this);'></div>";
        cell3.innerHTML = "<label id='" + object.id + "' output_ext='%'></label>";
        cell4.innerHTML = "<div><input id='switch-" + object.id + "' name='" + object.id + "' class='switch switch-btn' type='checkbox' onclick='update(this);'><label for='switch-" + object.id + "'></label></div>";
        //cell4.innerHTML = "<div class='switch'><input id='btn-toggle-"+object.id+"' name='"+object.id+"' class='btn-toggle btn-toggle-round' type='checkbox' onclick='update(this);'><label for='btn-toggle-"+object.id+"'></label></div>";
        setSlider(object.id, object.value);
        setButton(object.id, object.state);
    }
}

function update(source) {
    //console.log("Send "+source.id+" to server "+source.getAttribute("type")+" "+source.value);

    var cmd = {};

    switch (source.getAttribute("type")) {
        case "range":
            cmd.parameter = source.id.substr(7); // remove 'slider_'
            cmd.value = source.value;
            break;
        case "checkbox":
            //name = source.id.substr(11); //remove 'btn-toggle-''
            //state = source.checked?"on":"off";
            cmd.command = source.id.substr(7);
            break;
        default: // type is null because it's a selector
            //name = source.id.substr(9); //remove 'selector_''
            cmd.parameter = 'LIGHT_MODE';
            cmd.value = source.value;
            cmd.button = source.id.substr(9); // remove 'selector_'
            break;
    }
    //console.log(source);
    //console.log(cmd);
    //console.log("*** NOT SENDING COMMAND ***");
    send_command(cmd);
}

function sliderchange(source) {
    setLabel(source.getAttribute('output_id'), source.value);
}

function setButton(id, isOn) {
    /*
    if ( (isOn != true && isOn != false) || Number.isInteger(isOn)){
    if (isOn != "enabled") {
        if (Number.isInteger(isOn))
        isOn = (isOn==1)?true:false;
        else
        isOn = (isOn=="on")?true:false;
    } else {
        //document.getElementById('btn-toggle-' + id).classList.add("btn-toggle-round_enabled");
    }
    }
    //NSF need to chance the button color if enadled is a sent state.
    if( (btn = document.getElementById('btn-toggle-' + id)) != null)
    btn.checked = isOn;
    */
    if ((source = document.getElementById('switch-' + id)) == null)
        return;

    if (isOn == "on") {
        source.classList.add('switch-btn-on');
        source.classList.remove('switch-btn-flash');
        source.classList.remove('switch-btn-enabled');
        source.checked = true;
    } else if (isOn == "enabled") {
        source.classList.remove('switch-btn-on');
        source.classList.remove('switch-btn-flash');
        source.classList.add('switch-btn-enabled');
        source.checked = true;
    } else if (isOn == "flash") {
        source.classList.remove('switch-btn-on');
        source.classList.add('switch-btn-flash');
        source.classList.remove('switch-btn-enabled');
        source.checked = true;
    } else { // Off
        source.classList.remove('switch-btn-on');
        source.classList.remove('switch-btn-flash');
        source.classList.remove('switch-btn-enabled');
        source.checked = false;
    }
}

function lock(source) {
    source.setAttribute('lock', 'true');
}

function unlock(source) {
    source.setAttribute('lock', 'false');
}

function setSlider(id, value) {
    slider = document.getElementById("slider_" + id);

    try {
        if (slider.getAttribute('lock') == 'true')
            return;
    } catch (e) {
        return;
    }

    slider.value = value;
    setLabel(slider.getAttribute('output_id'), value);
}

function setLabel(id, value) {
    if ((label = document.getElementById(id)) != null) {
        if (value != -999 && value != " ")
            label.innerHTML = value + label.getAttribute('output_ext');
        else
            label.innerHTML = "--";
    }

}


function get_appropriate_ws_url() {
    var pcol;
    var u = document.URL;
    /*
     * We open the websocket encrypted if this page came on an
     * https:// url itself, otherwise unencrypted
     */
    if (u.substring(0, 5) == "https") {
        pcol = "wss://";
        u = u.substr(8);
    } else {
        pcol = "ws://";
        if (u.substring(0, 4) == "http")
            u = u.substr(7);
    }
    u = u.split('/');
    //alert (pcol + u[0] + ":6500");
    return pcol + u[0];
}
/* dumb increment protocol */
var socket_di;

function startWebsockets() {
    socket_di = new WebSocket(get_appropriate_ws_url());
    try {
        socket_di.onopen = function() {
            // success!
            get_devices();
        }
        socket_di.onmessage = function got_packet(msg) {
            document.getElementById("title").classList.remove("error");
            var data = JSON.parse(msg.data);
            if (data.type == 'status') {
                update_status(data);
            } else if (data.type == 'devices') {
                check_devices(data);
            }
        }
        socket_di.onclose = function() {
            // something went wrong
            document.getElementById("title").innerHTML = '  !!! Connection error !!!  '
            document.getElementById("title").classList.add("error");
            // Try to reconnect every 5 seconds.
            setTimeout(function() {
                startWebsockets();
            }, 5000);
        }
    } catch (exception) {
        alert('<p>Error' + exception);
    }
}

function get_devices() {
    var msg = {
        command: "GET_DEVICES"
    };
    socket_di.send(JSON.stringify(msg));
}

function send_command(cmd) {
    socket_di.send(JSON.stringify(cmd));
}

function reset() {
    socket_di.send("reset\n");
}

/*
 This is where everything starts!
*/

window.addEventListener('load', init);

function init() {
    getSize();
    startWebsockets();
}
