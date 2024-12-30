let effects = {};

const GET_EFFECTS_BYTE = 0xFF;

const PARAM_TYPE_UINT8 = 0x00;
const PARAM_TYPE_UINT32 = 0x01;
const PARAM_TYPE_INT8 = 0x02;
const PARAM_TYPE_INT32 = 0x03;
const PARAM_TYPE_FLOAT = 0x04;
const PARAM_TYPE_COLOR = 0x05;
const PARAM_TYPE_BOOLEAN = 0x06;

const paramTypes = {
    [PARAM_TYPE_INT8]: {
        inputType: "number",
        encode: encodeInt8,
        decode: decodeInt8,
        size: 1
    },
    [PARAM_TYPE_UINT8]: {
        inputType: "number",
        encode: encodeInt8,
        decode: decodeInt8,
        size: 1
    },
    [PARAM_TYPE_INT32]: {
        inputType: "number",
        encode: encodeInt32,
        decode: decodeInt32,
        size: 4
    },
    [PARAM_TYPE_UINT32]: {
        inputType: "number",
        encode: encodeInt32,
        decode: decodeInt32,
        size: 4
    },
    [PARAM_TYPE_FLOAT]: {
        inputType: "number",
        encode: encodeFloat,
        decode: decodeFloat,
        size: 4
    },
    [PARAM_TYPE_COLOR]: {
        inputType: "color",
        encode: encodeColor,
        decode: decodeColor,
        size: 3
    },
    [PARAM_TYPE_BOOLEAN]: {
        inputType: "checkbox",
        encode: encodeInt8,
        decode: decodeInt8,
        size: 1
    }
};

$(document).ready(async () => {
    await getEffects();

    for (let effect in effects) {
        $('#effect-select').append(`<option value="${effects[effect].id}">${effects[effect].name}</option>`);
    }

    $("#effect-select").on("change", async () => {
        let effectId = $("#effect-select").val();
        constructEffect(effectId);
        await loadEffect(effects[effectId]);
    });

    $('#upload-form').submit(e => {
        setUploadStatus("Uploading...", "info");

        e.preventDefault();

        data = new FormData();
        data.append('file', $('#file')[0].files[0]);

        $.ajax({
            url: 'http://tree.local/flash',
            type: 'post',
            data,
            timeout: 60000,
            processData: false,
            success:function() {
                setUploadStatus("Upload complete", "success");
            },
            error: function() {
                setUploadStatus("Upload failed", "error");
            }
        });
    });
});

function setStatus(element, message, messageType) {
    switch (messageType) {
        case 'info':
            element.prop("class", "info-text");
            break;
        case 'error':
            element.prop("class", "error-text");
            break;
        case 'success':
            element.prop("class", "success-text");
            break;
    }

    element.text(message);
}

function setUploadStatus(message, messageType) {
    let uploadStatus = $('#upload-status > p');
    setStatus(uploadStatus, message, messageType);
}

function setEffectStatus(message, messageType) {
    let effectStatus = $('#effect-status > p');
    setStatus(effectStatus, message, messageType);
}

function constructEffect(effectId) {
    let effect = effects[effectId];

    let effectParams = $('#effect-params');

    effectParams.css("grid-template-rows", `repeat(${effect.params.length}, 30px)`);

    effectParams.empty();

    if (effect.params.length == 0) {
        effectParams.css("display", "none");
    } else {
        effectParams.css("display", "grid");
    }

    for (let param of effect.params) {
        let paramLabel = `<label for="param-${param.name}">${param.name}</label>`;
        let inputType = paramTypes[param.type].inputType;
        let paramInput = `<input type="${inputType}" id="param-${param.name}" name="param-${param.name}">`;

        effectParams.append(paramLabel);
        effectParams.append(`<div class="center-flex">${paramInput}<\div>`);
    }
}

async function send_uart(data) {
    return new Uint8Array(await $.ajax({
        type: 'POST',
        timeout: 2000,
        url: 'http://tree.local/uart',
        contentType: 'application/octet-stream',
        data: new Uint8Array(data),
        processData: false,
        xhrFields: {
            responseType: "arraybuffer"
        },
    }));
}

async function send_save(data) {
    await $.ajax({
        type: 'POST',
        timeout: 2000,
        url: 'http://tree.local/save',
        contentType: 'application/octet-stream',
        data: new Uint8Array(data),
        processData: false,
    });
}

async function get_save(effect) {
    return new Uint8Array(await $.ajax({
        type: 'POST',
        timeout: 2000,
        url: 'http://tree.local/load',
        contentType: 'application/octet-stream',
        data: new Uint8Array([effect.id, effect.version]),
        processData: false,
        xhrFields: {
            responseType: "arraybuffer"
        },
    }));
}

function parseEffect(response, effects, i) {
    let id = response[i ++];
    let version = response[i ++];
    let name_len = response[i ++];

    let name = "";
    for (let k = 0; k < name_len; k ++) {
        name += String.fromCharCode(response[i ++]);
    }

    let params = [];

    i = parseParams(response, params, i);
    
    effects[id] = {
        id: id,
        version: version,
        name: name,
        params: params
    };

    return i;
}

function parseParams(response, params, i) {
    let n_params = response[i ++];

    for (let k = 0; k < n_params; k ++) {
        let param_name_len = response[i ++];
        let param_name = "";

        for (let l = 0; l < param_name_len; l ++) {
            param_name += String.fromCharCode(response[i ++]);
        }

        let param_type = response[i ++];

        params.push({
            name: param_name,
            type: param_type
        });
    }

    return i;
}

async function getEffects() {
    let response = await send_uart([GET_EFFECTS_BYTE]);

    let i = 0;

    let num_effects = response[i ++];
    
    for (let j = 0; j < num_effects; j ++) {
        i = parseEffect(response, effects, i);
    }
}

function colorToInt(color) {
    let r = parseInt(color.substr(1, 2), 16);
    let g = parseInt(color.substr(3, 2), 16);
    let b = parseInt(color.substr(5, 2), 16);

    return (r << 16) | (g << 8) | b;
}

function decodeInt8(bytes, i) {
    return bytes[i];
}

function decodeInt32(bytes, i) {
    return (bytes[i] << 24) | (bytes[i + 1] << 16) | (bytes[i + 2] << 8) | bytes[i + 3];
}

function decodeFloat(bytes, i) {
    let floatBytes = new Uint8Array([bytes[i], bytes[i + 1], bytes[i + 2], bytes[i + 3]]).buffer;
    return new Float32Array(floatBytes)[0];
}

function decodeColor(bytes, i) {
    let r = bytes[i];
    let g = bytes[i + 1];
    let b = bytes[i + 2];

    console.log(r, g, b);
    console.log(`#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`);
    return `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
}

function encodeInt8(value, bytes) {
    bytes.push(value & 0xFF);
}

function encodeInt32(value, bytes) {
    bytes.push((value >> 24) & 0xFF);
    bytes.push((value >> 16) & 0xFF);
    bytes.push((value >> 8) & 0xFF);
    bytes.push(value & 0xFF);
}

function encodeFloat(value, bytes) {
    let floatBytes = new Float32Array([value]).buffer;
    let floatArray = new Uint8Array(floatBytes);

    for (let byte of floatArray) {
        bytes.push(byte);
    }
}

function encodeColor(value, bytes) {
    value = colorToInt(value);

    bytes.push((value >> 16) & 0xFF);
    bytes.push((value >> 8) & 0xFF);
    bytes.push(value & 0xFF);
}

function encodeEffect(effectId) {
    let effect = effects[effectId];
    let bytes = [];

    bytes.push(effect.id);
    bytes.push(effect.version);
    
    for (let param of effect.params) {
        let value = $(`#param-${param.name}`).val();
        let encode = paramTypes[param.type].encode;

        encode(value, bytes);
    }

    return bytes;
}

async function sendEffect() {
    setEffectStatus("Sending effect...", "info");
    let effectId = $("#effect-select").val();

    let bytes = encodeEffect(effectId);
    let response;

    try {
        response = await send_uart(bytes);
    } catch (e) {
        setEffectStatus("Failed to send effect", "error");
        return;
    }


    if (response.length != 1 || response[0] != 0x00) {
        if (response.length == 1) {
            setEffectStatus("Request returned error code " + response[0], "error");
        } else {
            setEffectStatus("Request returned unexpected response", "error");
        }
    } else {
        setEffectStatus("Effect sent", "success");
    }
}

async function saveEffect() {
    setEffectStatus("Saving effect...", "info");
    let effectId = $("#effect-select").val();

    let bytes = encodeEffect(effectId);

    try {
        await send_save(bytes);
    } catch (e) {
        setEffectStatus("Failed to save effect", "error");
        return;
    }

    setEffectStatus("Effect saved", "success");
}

async function loadEffect(effect) {
    setEffectStatus("Loading effect...", "info");

    try {
        let save = await get_save(effect);

        let i = 2;

        for (let param of effect.params) {
            let decode = paramTypes[param.type].decode;
            let value = decode(save, i);

            $(`#param-${param.name}`).val(value);

            i += paramTypes[param.type].size;
        }
        
    } catch (e) {
        setEffectStatus("Effect has no saved default", "info");
        return;
    }

    setEffectStatus("Effect loaded", "success");
}

async function reset() {
    setEffectStatus("Resetting...", "info");

    try {
        await $.ajax({
            type: 'POST',
            timeout: 2000,
            url: 'http://tree.local/reset',
        });
    } catch (e) {
        setEffectStatus("Failed to reset", "error");
        return;
    }

    setEffectStatus("Reset complete", "success");
}