//'use strict';
var _lightProgramDropdown = false;
var _pressEvent;
var _ignoreMouseEvent = false;
var _temperature_units;
var _aqualink_data;
var _landscape = false;
var _displayNames = [];

/*
 This is where everything starts!
*/

window.addEventListener('load', init);
document.getElementById("header").addEventListener("click", showBackground);
document.getElementById("name").addEventListener("click", nameOnClick);
document.getElementById("thermostat_options_pane").addEventListener("click", haltEventPropogation);
document.getElementById("swg_options_pane").addEventListener("click", haltEventPropogation);
document.getElementById("pswitch_options_pane").addEventListener("click", haltEventPropogation);
document.getElementById("pswitch_option_switch").addEventListener("change", updatePwsitchOptions(this));

function nameOnClick()
{
    showVersion(this);
    event.stopPropagation();
}

function haltEventPropogation(e)
{
    if (!e)
      e = window.event;

    //IE9 & Other Browsers
    if (e.stopPropagation) {
      e.stopPropagation();
    }
    //IE8 and Lower
    else {
      e.cancelBubble = true;
    }
}

function init() {
    setSizeSpecifics();
    populateLightProgram();
    document.getElementById('thermostat_options').classList.remove("hide");
    document.getElementById('swg_options').classList.remove("hide");
    document.getElementById('pswitch_options').classList.remove("hide");
    setColors();
    load_background();
    showTileOptions(false);
    startWebsockets();
    resetBackgroundSize();
}

function populateLightProgram() {
    if (_lightProgramDropdown) {
        option = document.getElementById('pswitch_option_list');
        try {
            fLen = light_program.length;
            for (i = 0; i < fLen; i++) {
                var opt = document.createElement('option');
                opt.value = i + 1;
                opt.innerHTML = light_program[i];
                option.appendChild(opt);
            }
        } catch (e) {}
    } else {
        var tbody = document.getElementById('pswitch_table').getElementsByTagName('tbody')[0];
        var html1 = '';
        var html2 = '';
        var fLen = light_program.length;
        var i;
        for (i = 0; i < fLen; i++) {
            if (light_program[i].endsWith(" - Show"))
                html2 = html2 + "<div class='option_radiocontainer'><label><span class='radio'><input type='radio' name='light_program' value='" + (i + 1) + "' onchange='updatePwsitchOptions(this);'><span class='option_radio-value' aria-hidden='true'></span></span>" + light_program[i].substr(0, (light_program[i].length - 7)) + "</label></div>";
            else
                html1 = html1 + "<div class='option_radiocontainer'><label><span class='radio'><input type='radio' name='light_program' value='" + (i + 1) + "' onchange='updatePwsitchOptions(this);'><span class='option_radio-value' aria-hidden='true'></span></span>" + light_program[i] + "</label></div>";
        }
        var row;
        row = tbody.deleteRow(2);
        row = tbody.insertRow(2);
        row.innerHTML = "<td align='center'>Solid Color</td><td align='center'>Light Show</td>";
        row = tbody.insertRow(3);
        row.innerHTML = "<td align='left' valign='top'>" + html1 + "</td><td align='left' valign='top'>" + html2 + "</td>";
    }
}

function setProp(name) {
    if (typeof window[name] !== 'undefined')
        document.documentElement.style.setProperty('--' + name, window[name]);
}

function setColors() {
    setProp("tile_on_background");
    setProp("tile_on_background-crap");
    setProp("body_background");
    setProp("body_text");
    setProp("options_pane_background");
    setProp("options_pane_bordercolor");
    setProp("options_slider_highlight");
    setProp("options_slider_lowlight");
    setProp("head_background");
    setProp("head_text");
    setProp("error_background");
    setProp("tile_background");
    setProp("tile_text");
    setProp("tile_on_background");
    setProp("tile_on_text");
    setProp("tile_status_text");
}

function setSizeSpecifics() {
    var w = Math.max(document.documentElement.clientWidth, window.innerWidth || 0);
    var h = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
    //alert(w+"x"+h+" = "+(w*h));
    // 375x724 = 271500
    if ((w * h) > 370000) { // 370000 is kind-a just guess
        document.documentElement.style.setProperty('--tile-width', '125px');
        document.documentElement.style.setProperty('--tile_icon-height', '45px');
        document.documentElement.style.setProperty('--tile_name-height', '42px');
        document.documentElement.style.setProperty('--tile_status-height', '15px');
        document.documentElement.style.setProperty('--tile_grid-gap', '20px');
        document.documentElement.style.setProperty('--tile_name-lineheight', '1.4');
    }
    if (w > h)
        _landscape = true;
    else
        _landscape = false;
    setOrientation(_landscape);
    /*
      Future change element sizes here if needed
    */
    window.addEventListener("orientationchange", function() {
        var w = Math.max(document.documentElement.clientWidth, window.innerWidth || 0);
        var h = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
        // Have no idea why this is reversed, but w is less then h in landscape when captured here.
        if (w < h)
            _landscape = true;
        else
            _landscape = false;
        setOrientation(_landscape);
        // Need to finish rendering before calling this, so wait a small time
        setTimeout(resetBackgroundSize, 100);
    }, false);
}

function setOrientation(isLandscape) {
    if (isLandscape) {
        //alert('lll')
        document.getElementById("td_name").width = '72px';
        document.getElementById("td_timedate").width = '72px';
    } else {
        document.getElementById("td_name").width = '72px';
        document.getElementById("td_timedate").width = '102px';
    }
}

function load_background() {
    // check for undevined AND defined but empty
    if (typeof background_url === 'undefined' || background_url === undefined)
        return;
    //alert("load");
    // call the notify_complete function when the image has loaded
    var image = new Image();
    image.onload = function() {
        document.getElementById("body_wrap").style.backgroundImage = "url('" + this.src + "')";
    };
    if (typeof background_reload !== 'undefined' && background_reload > 0) {
        image.src = background_url + '?' + new Date().getTime();
        setTimeout(load_background, (background_reload * 1000));
    } else {
        image.src = background_url;
    }
}
// If the grid continer is larger than the page container, increase page so background looks nice
// also reduce size if needed on page rotate.
function resetBackgroundSize() {
    if (typeof background_url === 'undefined' || background_url === undefined)
        return;
    try {
        //inside = document.getElementById('wrapper');
        outside = document.getElementById('body_wrap');
        insideH = parseInt(window.getComputedStyle(document.getElementById('wrapper')).height);
        outsideH = parseInt(window.getComputedStyle(outside).height);
        var w = Math.max(document.documentElement.clientWidth, window.innerWidth || 0);
        var h = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
        if (insideH > outsideH || outsideH > h) {
            //outside.style.height = ((insideH > h)?(insideH + 40)+'px':(h + 40)+'px');
            outside.style.height = ((insideH > h) ? (insideH + 40) + 'px' : h + 'px');
            //alert("w:"+w+" h:"+h+" inH:"+insideH+" outH:"+outsideH+"\nUsed:"+outside.style.height);
        }
    } catch (e) {}
}

function endPress(id) {
    //pressEvent = 0;
    //alert ("Long press "+id);
    //switchTileState(id, true);
    showTileOptions(true, id);
}

function add_clickEvent(tile, id) {
    tile.addEventListener('touchstart', function() {
        _ignoreMouseEvent = true;
        _pressEvent = window.setTimeout(function() {
            _pressEvent = 0;
            endPress(id);
        }, 1000);
    });
    tile.addEventListener('touchend', function() {
        if (_pressEvent != 0) {
            clearTimeout(_pressEvent);
            switchTileState(id, false);
        }
    });
    tile.addEventListener("mousedown", function() {
        if (_ignoreMouseEvent == false) {
            _pressEvent = window.setTimeout(function() {
                _pressEvent = 0;
                endPress(id);
            }, 1000);
        }
    });
    tile.addEventListener("mouseup", function() {
        if (_pressEvent != 0 && _ignoreMouseEvent == false) {
            clearTimeout(_pressEvent);
            switchTileState(id, false);
        }
    });
}

function add_tile(id, name, status, type, subtype, off_imgurl, on_imgurl) {
    var height = getComputedStyle(document.documentElement).getPropertyValue('--tile_icon-height');
    var div = document.createElement('div');
    div.setAttribute('class', 'tile');
    div.setAttribute('id', id);
    div.setAttribute('type', subtype);
    //div.setAttribute('onclick', "switchTileState('" + id + "')");
    var subdiv = document.createElement('div');
    subdiv.setAttribute('class', 'tile_icon');
    subdiv.setAttribute('id', id + '_icon');
    if (off_imgurl != null) {
        var imgdiv = document.createElement('img');
        imgdiv.setAttribute('height', height + 'px');
        imgdiv.setAttribute('src', off_imgurl);
        subdiv.appendChild(imgdiv);
        div.appendChild(subdiv);
        if (on_imgurl != null) {
            subdiv = document.createElement('div');
            subdiv.setAttribute('class', 'tile_icon');
            subdiv.setAttribute('id', id + '_icon_on');
            imgdiv = document.createElement('img');
            imgdiv.setAttribute('height', height + 'px');
            imgdiv.setAttribute('src', on_imgurl);
            subdiv.style.display = 'none';
            subdiv.appendChild(imgdiv);
            div.appendChild(subdiv);
        }
    } else if (type == "value" || type == "thermostat") {
        var valdiv = document.createElement('div');
        valdiv.setAttribute('class', 'tile_icon_value disabled');
        valdiv.setAttribute('id', id + '_tile_icon_value');
        valdiv.textContent = '--';
        subdiv.appendChild(valdiv);
        div.appendChild(subdiv);
    }
    /*else if (type == "thermostat") {
           valdiv = document.createElement('div');
           valdiv.setAttribute('class', 'tile_icon_value disabled');
           valdiv.setAttribute('id', id + '_tile_icon_value');
           valdiv.textContent = '--';
           subdiv.appendChild(valdiv);
         } */
    subdiv = document.createElement('div');
    subdiv.setAttribute('class', 'tile_name');
    subdiv.setAttribute('id', id + '_name');
    subdiv.textContent = name;
    div.appendChild(subdiv);
    // Value and Thermostat are the same, but value is read only and no state
    if (type != "value") {
        subdiv = document.createElement('div');
        subdiv.setAttribute('class', 'tile_status');
        subdiv.setAttribute('id', id + '_status');
        subdiv.textContent = formatSatus(status);
        div.appendChild(subdiv);
        if (type == "switch" && subtype != "switch_program") {
            div.setAttribute('onclick', "switchTileState('" + id + "')");
        } else /*if (id != "SWG/Percent")*/ {
            add_clickEvent(div, id);
        }
    }
    document.getElementById('wrapper').appendChild(div);
}

function switchTileState(id, details) {
    if (details) {
        console.log("TILE DETAILS WHAY ARE WE HERE??? '" + id + "'");
    } else {
        setTileState(id, (document.getElementById(id).getAttribute('status') == 'off'))
    }
}

function setTileState(id, state) {
    try {
        if (state == (document.getElementById(id).getAttribute('status') == 'off')) {
            send_command(id);
            console.log("Switch state " + id + " to " + (state) ? "on" : "off");
        } else {
            console.log("state " + id + " to " + (state) ? "on" : "off")
        }
        setTileOn(id, ((state) ? "on" : "off"));
    } catch (exception) {}
}

function setThermostatSetpoint(id, sp_value) {
    var tile;
    if ((tile = document.getElementById(id)) == null) {
        return;
    }
    tile.setAttribute('setpoint', sp_value);
    send_setpoint(id);
}

function setTileValue(id, value) {
    var ext = '';
    if (value == undefined || value.startsWith("-999") || value.startsWith(" "))
        value = '--';
    else {
        try {
            if ((type = document.getElementById(id).getAttribute('type')) != null) {
                if (type == 'temperature' || type == 'setpoint_thermo' || type == 'setpoint_freeze')
                    ext = '&deg;';
                else if (type == 'setpoint_swg')
                    ext = '%';
            }
        } catch (e) {
            //console.log('ERROR id=' + id + ' Line 764 | element = '+document.getElementById(id));
        }
    }
    //document.getElementById(id + '_tile_icon_value').textContent = value;
    var tile;
    if ((tile = document.getElementById(id + '_tile_icon_value')) != null)
        tile.innerHTML = value + ext;
}

function setTileOnText(id, text) {
    try {
        if (document.getElementById(id).getAttribute('status') == 'on') {
            document.getElementById(id + '_status').innerHTML = text;
        } else {
            console.log("Tile " + id + " status is '" + document.getElementById(id).getAttribute('status') + "' not setting text to '" + text + "'");
        }
    } catch (exception) {}
}

function setThermostatTile(id, value, sp_value) {
    setTileValue(id, value);
    var tile;
    if ((tile = document.getElementById(id)) != null) {
        tile.setAttribute('setpoint', sp_value);
    }
}

function setTileAttribute(id, attribute, value) {
    var tile;
    if ((tile = document.getElementById(id)) != null) {
        //console.log("Set Attribute "+attribute+ " to "+value+" for "+id);
        tile.setAttribute(attribute, value);
    }
}

function getTileAttribute(id, attribute) {
    var tile;
    if ((tile = document.getElementById(id)) != null) {
        //console.log("get Attribute "+attribute+ " to "+tile.getAttribute(attribute)+" for "+id);
        return tile.getAttribute(attribute);
    }
    return null;
}

function formatSatus(status) {
    var index;
    if ((index = status.indexOf("AUX")) >= 0) {
        aux = status.substr(index, 4);
        status = status.charAt(0).toUpperCase() + status.substr(1).toLowerCase();
        if ((name = _displayNames[aux]) != undefined) {
            status = status.substr(0, index) + name + status.substr(index + 4);
        }
    } else if ((index = status.indexOf("ERROR")) >= 0) {
        status = status;
    } else {
        status = status.charAt(0).toUpperCase() + status.substr(1).toLowerCase();
    }
    return status;
}

function setTileOn(id, status) {
    //function setTileOn(id, status, text) {
    var tile;
    var text;
    if ((tile = document.getElementById(id)) == null) {
        //console.log("Error unknown ID '"+id+"' can't set to '"+status+"'");
        return;
    }
    if (status != "on" && status != "off" && status != "enabled" && status != "flash") {
        console.log("Error unknown status '" + status + "' for '" + id + "'");
        return;
    }
    tile.setAttribute('status', status);
    if (status != 'off') {
        if (status == 'flash') {
            tile.classList.add("flash");
            tile.classList.remove("on");
            text = "Delay";
        } else {
            tile.classList.add("on");
            tile.classList.remove("flash");
            text = "On";
        }
        var offimg;
        var onimg;
        if ((offimg = document.getElementById(id + '_icon')) != null &&
            (onimg = document.getElementById(id + '_icon_on')) != null) {
            offimg.style.display = 'none';
            onimg.style.display = 'table';
        }
        var type;
        if ((type = tile.getAttribute('type')) != null) {
            if (type == 'setpoint_swg') {
                if (tile.getAttribute('Boost') == 'on')
                    text = "Boost";
                else if (status == 'enabled')
                    text = 'Enabled';
                else
                    text = 'Generating';
            } else if (type == 'setpoint_thermo')
                if (status == 'enabled')
                    text = 'Heat to ' + tile.getAttribute('setpoint');
                else
                    text = 'Heating to ' + tile.getAttribute('setpoint');
            else if (type == 'setpoint_freeze')
                text = 'Turn on ' + tile.getAttribute('setpoint') + "&deg;";
            //else
            //  text = "On";
        }
        //document.getElementById(id + '_status').innerHTML = "On";
    } else {
        tile.classList.remove("on");
        tile.classList.remove("flash");
        if ((offimg = document.getElementById(id + '_icon')) != null &&
            (onimg = document.getElementById(id + '_icon_on')) != null) {
            onimg.style.display = 'none';
            offimg.style.display = 'table';
        }
        text = "Off";
    }
    document.getElementById(id + '_status').innerHTML = text;
    var tile_icon = document.getElementById(id + '_tile_icon_value');
    type = tile.getAttribute('type');
    if (status != null && tile_icon != null) {
        if (status == 'enabled' || status == 'flash') {
            tile_icon.classList.add("enabled");
            tile_icon.classList.remove("disabled");
            tile_icon.classList.remove("cool");
            tile_icon.classList.remove("heat");
        } else if (status == 'off') {
            tile_icon.classList.remove("enabled");
            tile_icon.classList.add("disabled");
            tile_icon.classList.remove("cool");
            tile_icon.classList.remove("heat");
        } else if (status == 'on' && type != 'setpoint_swg' && type != 'setpoint_freeze') {
            tile_icon.classList.remove("enabled");
            tile_icon.classList.remove("disabled");
            tile_icon.classList.remove("cool");
            tile_icon.classList.add("heat");
        } else if (status == 'on') {
            tile_icon.classList.remove("enabled");
            tile_icon.classList.remove("disabled");
            tile_icon.classList.add("cool");
            tile_icon.classList.remove("heat");
        }
    }
}

function createTile(object) {
    if (object.name == 'NONE') {
        return;
    }
    if (typeof devices !== 'undefined' && devices.indexOf(object.id) < 0) {
        return;
    }
    if (object.type == 'switch' || object.type == 'switch_program') {
        var img = object.id.replace('/', '_');
        add_tile(object.id, object.name, object.state, 'switch', object.type, 'images/' + img + '-off.png', 'hk/' + img + '-on.png');
        setTileOn(object.id, object.status, null);
        if (typeof object.Pump_RPM !== 'undefined' && object.Pump_RPM) {
            setTileOnText(object.id, 'RPM:' + object.Pump_RPM);
        }
    } else if (object.type == 'value' || object.type == 'temperature') {
        add_tile(object.id, object.name, object.state, 'value', object.type);
        setTileValue(object.id, object.value);
    } else if (object.type == 'setpoint_thermo' || object.type == 'setpoint_swg' || object.type == 'setpoint_freeze') {
        add_tile(object.id, object.name, object.state, 'thermostat', object.type);
        document.getElementById(object.id).setAttribute('setpoint', object.spvalue);
        setTileValue(object.id, object.value);
        setTileOn(object.id, object.status);
    } else {
        //console.log("Unknown Device");   
    }
}

function showTileOptions(show, id, contex) {
    var active_option;
    if (show == true) {
        var wrapH = document.getElementById('wrapper').clientHeight + 'px';
        if (id != null && document.getElementById(id).getAttribute('type') == 'setpoint_thermo') {
            active_option = document.getElementById('thermostat_options');
            document.getElementById('swg_options').style.display = 'none';
            document.getElementById('pswitch_options').style.display = 'none';
        } else if (id != null && document.getElementById(id).getAttribute('type') == 'switch_program') {
            active_option = document.getElementById('pswitch_options');
            document.getElementById('thermostat_options').style.display = 'none';
            document.getElementById('swg_options').style.display = 'none';
        } else /*if (id != null && document.getElementById(id).getAttribute('type') == 'setpoint_swg')*/ {
            active_option = document.getElementById('swg_options');
            document.getElementById('thermostat_options').style.display = 'none';
            document.getElementById('pswitch_options').style.display = 'none';
        }
        active_option.style.display = 'flex';
        var optionH = window.getComputedStyle(active_option, null).getPropertyValue("height");
        if (optionH <= wrapH)
            active_option.style.height = wrapH;

        //console.log("Option:"+optionH+" wrapH:"+wrapH);
        document.getElementById('wrapper').classList.add("opaque");
    } else {
        // Fake onclick to close and save any values if open
        if (contex == null) {
            if (document.getElementById('thermostat_options').style.display == 'flex')
                document.getElementById("options_close").click();
            else if (document.getElementById('swg_options').style.display == 'flex')
                document.getElementById("swg_options_close").click();
            else if (document.getElementById('pswitch_options').style.display == 'flex')
                document.getElementById("pswitch_options_close").click();
        }
        document.getElementById('thermostat_options').style.display = 'none';
        document.getElementById('swg_options').style.display = 'none';
        document.getElementById('pswitch_options').style.display = 'none';
        document.getElementById('wrapper').classList.remove("opaque");
        return;
    }
    if (show == true) {
        var tile = document.getElementById(id);
        var sp_value = tile.getAttribute('setpoint');
        //tile_state = tile.classList.contains("on");
        var tile_state = !(tile.getAttribute('status') == 'off');
        var type = tile.getAttribute('type');
        var slider;
        var slider_output;
        var title;
        var close_button;
        var ext;
        var oswitch;
        if (type == 'setpoint_swg') {
            slider = document.getElementById("swg_option_slider_range");
            slider_output = document.getElementById("swg_option_slider_text_value");
            title = document.getElementById("swg_option_title");
            close_button = document.getElementById("swg_options_close");
            ext = "%";
        } else if (type == 'setpoint_freeze') {
            slider = document.getElementById("swg_option_slider_range");
            slider_output = document.getElementById("swg_option_slider_text_value");
            title = document.getElementById("swg_option_title");
            close_button = document.getElementById("swg_options_close");
            ext = '&deg;' + _temperature_units;
        } else if (type == 'switch_program') {
            //slider = document.getElementById("pswitch_option_slider_range");
            //slider_output = document.getElementById("pswitch_option_slider_text_value");
            title = document.getElementById("pswitch_option_title");
            close_button = document.getElementById("pswitch_options_close");
            //ext = '&deg;' + _temperature_units;
        } else {
            slider = document.getElementById("option_slider_range");
            slider_output = document.getElementById("option_slider_text_value");
            title = document.getElementById("option_title");
            close_button = document.getElementById("options_close");
            ext = '&deg;' + _temperature_units;
        }
        if (type == 'setpoint_swg') {
            slider.min = 0;
            slider.max = 101;
            slider.step = 5;
        } else if (type == 'setpoint_freeze') {
            if (_temperature_units != 'c') { // Change to DegF
                slider.min = 34;
                slider.max = 42;
                slider.step = 1;
            } else { // Change to DegF
                slider.min = 1;
                slider.max = 5;
                slider.step = 1;
            }
        } else if (type == 'setpoint_thermo') {
            if (_temperature_units != 'c') { // Change to DegF
                slider.min = 36;
                slider.max = 104;
                slider.step = 1;
            } else { // Change to DegF
                slider.min = 0;
                slider.max = 40;
                slider.step = 1;
            }
        }
        title.innerHTML = document.getElementById(id + '_name').innerHTML;
        if (type == 'switch_program') {
            var pswitch = document.getElementById("pswitch_option_list");
            oswitch = document.getElementById("pswitch_option_switch");
            var oswitch_output = document.getElementById("pswitch_option_switch_text_value");
            oswitch.checked = tile_state;
            oswitch_output.innerHTML = ((oswitch.checked) ? "On" : "Off");
            oswitch.onclick = function() {
                oswitch_output.innerHTML = ((oswitch.checked) ? "On" : "Off");
            }
        } else if (type == 'setpoint_swg') {
            slider.value = sp_value;
            oswitch = document.getElementById("swg_option_switch");
            //oswitch.checked = tile_state;
            //console.log("Boost attribute " + getTileAttribute(id, "boost") + " for "+id);
            oswitch.checked = getTileAttribute(id, "Boost") == 'on' ? true : false;
            var oswitch_output = document.getElementById("swg_option_switch_text_value");
            slider_output.innerHTML = slider.value + ext;
            oswitch_output.innerHTML = ((oswitch.checked) ? "Boost On" : "Boost Off");
            slider.oninput = function() {
                slider_output.innerHTML = this.value + ext;
                //sp_value = this.value
            }
            oswitch.onclick = function() {
                oswitch_output.innerHTML = ((oswitch.checked) ? "Boost On" : "Boost Off");
                //setTileOn(id, ((oswitch.checked)?"on":"off"), null);
            }
        } else {
            slider.value = sp_value;
            oswitch = document.getElementById("option_switch");
            oswitch.checked = tile_state;
            var oswitch_output = document.getElementById("option_switch_text_value");
            slider_output.innerHTML = slider.value + ext;
            oswitch_output.innerHTML = ((oswitch.checked) ? "Heat" : "Off");
            slider.oninput = function() {
                slider_output.innerHTML = this.value + ext;
                //sp_value = this.value
            }
            oswitch.onclick = function() {
                oswitch_output.innerHTML = ((oswitch.checked) ? "Heat" : "Off");
                //setTileOn(id, ((oswitch.checked)?"on":"off"), null);
            }
        }
        var cTime;
        try {
            cTime = performance.now();
        } catch (e) {
            cTime = Date.now(); // NSF Probably won't test well below, need to check.
        }
        var clickHandler = function(e) {
            // Short time diff means event was the one that launched the options pane, so ignore
            if ((e.timeStamp - cTime) > 1000)
                showTileOptions(false);
        };
        document.getElementById("body_wrap").addEventListener("click", clickHandler);
        close_button.onclick = function() {
            document.getElementById("body_wrap").removeEventListener("click", clickHandler);
            var state = oswitch.checked;
            if (type == 'switch_program') {
                var mode = false;
                if (_lightProgramDropdown) {
                    if (pswitch.selectedIndex > 0) {
                        send_light_mode(pswitch.selectedIndex, id);
                        mode = true;
                    }
                } else {
                    var radio = document.getElementsByName("light_program");
                    var x;
                    for (x = 0; x < radio.length; x++) {
                        if (radio[x].checked == true) {
                            send_light_mode(radio[x].value, id);
                            mode = true;
                            break;
                        }
                    }
                }
                if (mode == false) {
                    // if (state == (tile.getAttribute('status') == 'off')) // Only bother with this if we didn;t set the light mode.
                    setTileState(id, state);
                }
            } else if (type == 'setpoint_swg') {
                //console.log ("Boost attribute = "+tile.getAttribute('boost'));
                //console.log ("state = "+state);
                if (state == (tile.getAttribute('Boost') == 'off')) {
                    //console.log ("change boost");
                    send_command('SWG/Boost', (state == true ? "on" : "off"));
                } else {
                    if (sp_value != slider.value && slider.value != 101) // Don't change setpoint if slider is on boost
                        setThermostatSetpoint(id, slider.value);
                }
            } else {
                var value = slider.value;
                if (state == (tile.getAttribute('status') == 'off'))
                    setTileState(id, state);
                if (sp_value != slider.value)
                    setThermostatSetpoint(id, slider.value)
            }
            //showTileOptions(false, null, 'this');
            active_option.style.display = 'none';
            document.getElementById('wrapper').classList.remove("opaque");
        }
    }
}

function update_status(data) {
    //console.log("Received Update Satus");
    _aqualink_data = data;
    var el = document.getElementById("datetime");

    if (data.time.toString() != "" && data.time.toString() != "xx")
        el.innerHTML = data.time.toLowerCase().replace(" ", "&nbsp;") + ((_landscape) ? "&nbsp;" : "<br>") + data.date.toLowerCase().replace(" ", "&nbsp;");

    if (document.getElementById("name").innerHTML == "AqualinkD") {
        if (data.status != "Ready") {
            document.getElementById("message").innerHTML = formatSatus(data.status);
        } else {
            document.getElementById("message").innerHTML = '';
        }
    }

    if (data.status.indexOf("ERROR") >= 0)
        document.getElementById("header").classList.add("error");

    // Hard code value updated as I can't be bothered to update source.
    // should come back and re-do these values.
    // The ID's should ultimatly come from json
    setThermostatTile("Pool_Heater", data.pool_temp, data.pool_htr_set_pnt);
    setThermostatTile("Spa_Heater", data.spa_temp, data.spa_htr_set_pnt);
    setThermostatTile("SWG", data.swg_percent, data.swg_percent);
    setThermostatTile("Freeze_Protect", data.air_temp, data.frz_protect_set_pnt);
    setTileValue("SWG/PPM", data.swg_ppm);
    setTileValue("Temperature/Air", data.air_temp);
    setTileValue("Temperature/Pool", data.pool_temp);
    setTileValue("Temperature/Spa", data.spa_temp);
    /*
    setTileValue("Air", data.air_temp);
    setTileValue("Pool_Water", data.pool_temp);
    setTileValue("Spa_Water", data.spa_temp);*/
    for (var obj in data.leds) {
        setTileOn(obj.toString(), data.leds[obj]);

        if (obj.toString() == 'SWG/Boost') {
            //console.log("BOOST IS "+data.leds[obj]);
            setTileAttribute('SWG', "Boost", data.leds[obj]);
        }
    }

    if (data.swg_boost_msg != null) {
        var tile = document.getElementById('SWG');
        if (tile != null) {
            var sp_value = tile.getAttribute('setpoint');
            if (sp_value == 101)
                document.getElementById('SWG' + '_status').innerHTML = 'Boost ' + data.swg_boost_msg;
        }
    }

    //obj.leds.'SWG/Boost' 

    // NSF Really quick hack to show RPM on filter pump. Need to come back and do this correctly.
    /*
    if ((typeof data.extra["Pump_1"] !== 'undefined') && (typeof data.extra["Pump_1"].RPM !== 'undefined')) {
      console.log(data.extra["Pump_1"].RPM);
      if (document.getElementById('Filter_Pump').getAttribute('status') == 'on') {
        document.getElementById('Filter_Pump_status').innerHTML = 'RPM:'+data.extra["Pump_1"].RPM;
      }
    }*/
    //console.log(data["Pump_1"].RPM);

    var i = 1;
    while (i < 5) {
        //console.log(data["Pump_"+i].RPM);
        if ((typeof data["Pump_" + i] !== 'undefined') && (typeof data["Pump_" + i].RPM !== 'undefined')) {
            setTileOnText(data["Pump_" + i].id, 'RPM:' + data["Pump_" + i].RPM);
            /*
            if (document.getElementById(data["Pump_"+i].id).getAttribute('status') == 'on')
              document.getElementById(data["Pump_"+i].id + '_status').innerHTML = 'RPM:'+data["Pump_"+i].RPM;
            else
              console.log(data["Pump_"+i].id + ' is off, not displaying RPM ' + data["Pump_"+i].RPM);*/
        }
        i++;
    }
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
    data['devices'].sort(deviceSort);
    for (var obj in data['devices']) {
        if (document.getElementById(data['devices'][obj].id) == null) {
            _displayNames[data['devices'][obj].id.replace("Aux_", "AUX")] = data['devices'][obj].name;
            createTile(data['devices'][obj]);
        } else {
            //console.log("old type "+document.getElementById(data['devices'][obj].id).getAttribute('type')+" | new "+data['devices'][obj].type);
            if (document.getElementById(data['devices'][obj].id).getAttribute('type') != data['devices'][obj].type) {
                //console.log("Remove and create "+data['devices'][obj].id);
                var element = document.getElementById(data['devices'][obj].id);
                element.parentNode.removeChild(element);
                createTile(data['devices'][obj]);
            }
        }
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
            // Set another load 1 minute from now just incase the server hasn't got all the devices yet
            window.setTimeout(get_devices, (60 * 1000));
        }
        socket_di.onmessage = function got_packet(msg) {
            document.getElementById("header").classList.remove("error");
            var data = JSON.parse(msg.data);
            if (data.type == 'status') {
                update_status(data);
            } else if (data.type == 'devices') {
                check_devices(data);
                resetBackgroundSize();
                //window.setTimeout(get_devices, (300 * 1000)); // Check for new dvices ever 5 mins.
                window.setTimeout(get_devices, (60 * 1000)); // Check for new dvices ever 1 mins.
            }
        }
        socket_di.onclose = function() {
            // something went wrong
            document.getElementById("message").innerHTML = '  Connection error!  '
            document.getElementById("header").classList.add("error");
            // Try to reconnect every 5 seconds.
            setTimeout(function() {
                startWebsockets()
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

function send_command(cmd, value = null) {
    var _cmd = {};
    _cmd.command = cmd;
    if (value != null)
        _cmd.value = value;
    socket_di.send(JSON.stringify(_cmd));
}

function send_setpoint(id) {
    var temperature = {};
    var tile;
    if ((tile = document.getElementById(id)) == null) {
        return;
    }
    temperature.parameter = tile.getAttribute('id');
    temperature.value = tile.getAttribute('setpoint');
    socket_di.send(JSON.stringify(temperature));
}

function send_light_mode(value, id) {
    console.log("Set light mode to " + value + " id=" + id);
    var mode = {};
    //mode.parameter = 'POOL_LIGHT_MODE';
    mode.parameter = 'LIGHT_MODE';
    mode.value = value;
    mode.button = id;
    socket_di.send(JSON.stringify(mode));
}

function reset() {
    socket_di.send("reset\n");
}

function showBackground() {
    wrapper = document.getElementById("wrapper");
    if (wrapper.classList.contains("vopaque")) {
        wrapper.classList.remove("vopaque");
    } else {
        wrapper.classList.add("vopaque");
    }
}

function showVersion(source) {
    if ((message = document.getElementById("message")) != null) {
        message.innerHTML = "AqualinkD : " + _aqualink_data.aqualinkd_version + " | AqualinkRS : " + _aqualink_data.version;
    }
    source.innerHTML = "Version(s)";
    setTimeout(function() {
        source.innerHTML = "AqualinkD";
    }, 5000);
}

function updatePwsitchOptions(source) {
    if (source.type == 'radio') {
        document.getElementById('pswitch_option_switch_text_value').innerHTML = light_program[source.value - 1];
        document.getElementById('pswitch_option_switch').checked = true;
    } else if (source.type == 'checkbox') {
        //console.log(source);
        if (document.getElementById('pswitch_option_switch').checked == false) {
            var radio = document.getElementsByName("light_program");
            for (x = 0; x < radio.length; x++) {
                radio[x].checked = false
            }
            document.getElementById('pswitch_option_switch_text_value').innerHTML = 'Off';
        }
    }
}