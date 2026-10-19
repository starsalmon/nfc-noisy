#include "web_config.h"

#if defined(NFCNOISY_WIFI_OTA)

#include "nfc_session.h"
#include "volume.h"
#include "volume_tags.h"

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

static WebServer s_server(80);
static bool s_started = false;

static void send_json_status() {
    String j = "{\"volume\":";
    j += volume_get_percent();
    j += ",\"last\":\"";
    j += nfc_session_last_uid();
    j += "\",\"up\":[";
    for (int i = 0; i < volume_tags_count(VolumeTagDir::Up); i++) {
        if (i) {
            j += ',';
        }
        j += '"';
        j += volume_tags_uid(VolumeTagDir::Up, i);
        j += '"';
    }
    j += "],\"down\":[";
    for (int i = 0; i < volume_tags_count(VolumeTagDir::Down); i++) {
        if (i) {
            j += ',';
        }
        j += '"';
        j += volume_tags_uid(VolumeTagDir::Down, i);
        j += '"';
    }
    j += "]}";
    s_server.send(200, "application/json", j);
}

static void handle_root() {
    const char *html = R"HTML(<!DOCTYPE html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>NFCNoisy</title>
<style>
body{font-family:system-ui,sans-serif;max-width:32rem;margin:1rem auto;padding:0 0.5rem}
label{display:block;margin:0.75rem 0 0.25rem}
input[type=range]{width:100%}
button{margin:0.25rem 0.25rem 0.25rem 0;padding:0.5rem 0.75rem}
.uid{font-family:monospace;word-break:break-all}
ul{padding-left:1.2rem}
</style></head><body>
<h1>NFCNoisy</h1>
<p>Last scanned UID: <span class="uid" id="last">—</span></p>
<p>Volume: <strong id="vpct">—</strong>%</p>
<label for="slider">Volume slider</label>
<input type="range" id="slider" min="0" max="100" step="5" onchange="setVol(this.value)">
<h2>Volume tags</h2>
<p>Tap a tag on the reader, then assign it here (UID only — works with Bambu spool tags).</p>
<button type="button" onclick="assign('up')">Assign last → Volume up</button>
<button type="button" onclick="assign('down')">Assign last → Volume down</button>
<h3>Volume up</h3><ul id="uplist"></ul>
<h3>Volume down</h3><ul id="dnlist"></ul>
<script>
async function refresh(){
  const r=await fetch('/api/status');const d=await r.json();
  document.getElementById('last').textContent=d.last||'(none)';
  document.getElementById('vpct').textContent=d.volume;
  document.getElementById('slider').value=d.volume;
  const mk=(id,arr,role)=>{const u=document.getElementById(id);u.innerHTML='';
    (arr||[]).forEach(uid=>{const li=document.createElement('li');
      li.innerHTML='<span class="uid">'+uid+'</span> <button type="button" onclick="rm(\''+role+'\',\''+uid+'\')">Remove</button>';
      u.appendChild(li);});};
  mk('uplist',d.up,'up');mk('dnlist',d.down,'down');
}
async function setVol(v){await fetch('/api/volume?pct='+encodeURIComponent(v),{method:'POST'});refresh();}
async function assign(role){await fetch('/api/assign?role='+role,{method:'POST'});refresh();}
async function rm(role,uid){await fetch('/api/remove?role='+role+'&uid='+encodeURIComponent(uid),{method:'POST'});refresh();}
refresh();setInterval(refresh,3000);
</script>
<p><small>One volume step per tag tap — lift the tag between changes.</small></p>
</body></html>)HTML";
    s_server.send(200, "text/html", html);
}

static void handle_set_volume() {
    if (s_server.hasArg("pct")) {
        volume_set_percent(s_server.arg("pct").toInt());
        volume_play_feedback();
    }
    s_server.send(200, "text/plain", "ok");
}

static void handle_assign() {
    const String role = s_server.arg("role");
    const char *uid = nfc_session_last_uid();
    if (!uid || uid[0] == '\0') {
        s_server.send(400, "text/plain", "no tag scanned yet");
        return;
    }
    if (role == "up") {
        volume_tags_add(VolumeTagDir::Up, uid);
    } else if (role == "down") {
        volume_tags_add(VolumeTagDir::Down, uid);
    } else {
        s_server.send(400, "text/plain", "bad role");
        return;
    }
    s_server.send(200, "text/plain", "ok");
}

static void handle_remove() {
    const String role = s_server.arg("role");
    const String uid = s_server.arg("uid");
    if (uid.length() == 0) {
        s_server.send(400, "text/plain", "uid required");
        return;
    }
    if (role == "up") {
        volume_tags_remove(VolumeTagDir::Up, uid.c_str());
    } else if (role == "down") {
        volume_tags_remove(VolumeTagDir::Down, uid.c_str());
    } else {
        s_server.send(400, "text/plain", "bad role");
        return;
    }
    s_server.send(200, "text/plain", "ok");
}

void web_config_on_wifi_ready() {
    if (s_started) {
        return;
    }
    s_server.on("/", handle_root);
    s_server.on("/api/status", send_json_status);
    s_server.on("/api/volume", HTTP_POST, handle_set_volume);
    s_server.on("/api/assign", HTTP_POST, handle_assign);
    s_server.on("/api/remove", HTTP_POST, handle_remove);
    s_server.begin();
    s_started = true;
    Serial.println("Web UI: http://nfcnoisy.local/");
}

void web_config_loop() {
    if (s_started) {
        s_server.handleClient();
    }
}

#else

void web_config_on_wifi_ready() {}
void web_config_loop() {}

#endif
