let g_fragmentZoom = 1.0/1.5;
let g_blurWidth;
let g_kernelSize;
let g_ratio;
let g_thresh;
let g_areaThresh;
let g_flushCache = true;


let g_leftSelected;
let g_rightSelected;

let g_lastusedClickAndSeeShapeRight;
let g_lastusedClickAndSeeShapeLeft;

let g_mainGlobalState;
let g_shapeResults;


function getHashDistance() {
    let databaseString = g_lastusedClickAndSeeShapeRight;
    let lookupString = g_lastusedClickAndSeeShapeLeft;

    if (databaseString === undefined || lookupString === undefined)
        return -1;

    let rotation = parseInt(document.getElementById("clickandseeRotation").value);

    let distance = module.getHashDistanceFromCanvas(
        databaseString, rotation,
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        lookupString,
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
    );
    document.getElementById("hammingDistanceForResult").innerHTML = "Hash distance: "+distance;
}



function drawshapefromClickandseeRight(shapeStr1, dist) {
    if (shapeStr1 === undefined)
        return;

    const can = getCleanUICanvas("databaseCanvas");
    //draw the shape
    drawPolyFull(can.ctx_ui, shapeStrToShape(shapeStr1), 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');
}

function drawshapefromClickandseeLeft(index) {
    // g_lastusedClickAndSeeShapeLeft = shapeStr1;

    // drawShapeAndFragmentClickAndSee(
    //     lookup_canvas_wasm_heap.ptr,
    //     lookup_canvas_wasm_heap.width,
    //     lookup_canvas_wasm_heap.height,
    //     shapeStr1, 256, "lookupCanvasFrag", rotation);
    //
    // getHashDistance();

    //draw this and get fill the other list
    const cur = g_shapeResults.shapes[parseInt(index)].shapes;

    const can = getCleanUICanvas("lookupCanvas");
    drawPolyFull(can.ctx_ui, shapeStrToShape(g_shapeResults.shapes[parseInt(index)].shape1), 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');

    const list = document.getElementById("clickandseeListRight");
    list.innerHTML = "";

    for (let i = 0; i < cur.length; i++)
    {
        let opt = "" + i;
        let el = document.createElement("div");
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromClickandseeRight('${cur[i].shape}', ${cur[i].dist})" id='clickandseeShapeListElm${i}'>-----${i} - ${cur[i].dist} (${cur[i].hashDist})</div>`;
        list.appendChild(el);
    }
    //now loop through all the shapes and populate the list
}

function drawshapefromClickandseeLeftRepeat(_shape1, _shape2) {
    const shape1 = shapeStrToShape(_shape1);
    const shape2 = shapeStrToShape(_shape2);
    //draw this and get fill the other list
    const can = getCleanUICanvas("lookupCanvas");
    drawPolyFull(can.ctx_ui, shape1, 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');
    const can2 = getCleanUICanvas("databaseCanvas");
    drawPolyFull(can2.ctx_ui, shape2, 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');
    //now loop through all the shapes and populate the list
}

function parseClickandseeShapesLeft(shapes) {
    if (shapes.length == 0) {
        return;
    }

    const list = document.getElementById("clickandseeListLeft");
    list.innerHTML = "";

    const can = getCleanUICanvas("lookupCanvas");
    for (let i = 0;i < shapes.shapes.length;i++) {
        let opt = ""+i;
        let el = document.createElement("div");
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromClickandseeLeft('${opt}', 0)" id='clickandseeShapeListElm${i}'>-----${i}</div>`;
        list.appendChild(el);

        drawPolyFull(can.ctx_ui, shapeStrToShape(opt), 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');
    }
    // if (lines.length > 0)
    //     drawshapefromClickandseeLeft(lines[0], 0);
}

function parseClickandseeShapesRight(shapes) {
    if (shapes.length == 0) {
        return;
    }
    const lines = shapes.split('\n').filter(function (el) {
        return el.length > 0;
    });
    const list = document.getElementById("clickandseeListRight");
    list.innerHTML = "";

    const can = getCleanUICanvas("databaseCanvas");
    for (let i = 0;i < lines.length;i++) {
        let opt = lines[i];
        let el = document.createElement("div");
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromClickandseeRight('${opt}', 0)" id='clickandseeShapeListElm${i}'>-----${i}</div>`;
        list.appendChild(el);

        drawPolyFull(can.ctx_ui, shapeStrToShape(opt), 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');
    }
    if (lines.length > 0)
        drawshapefromClickandseeRight(lines[0], 0);
}

function getShapeWithPointInsideLeft(x, y) {
    const shapeStr = module.getShapeWithPointInside(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        x, y, g_thresh, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);

    parseClickandseeShapesLeft(shapeStr);

    return shapeStr;
}

function getShapeWithPointInsideRight(x, y) {
    const shapeStr = module.getShapeWithPointInside(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        x, y, g_thresh, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);

    parseClickandseeShapesRight(shapeStr);

    return shapeStr;
}

function getShapeWithPointInsideCommon(e, canvasId, getShapeWithPointInsideFunc) {
    var canvasElem = $("#"+canvasId+"_ui")[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    let canvasElemObj = getCleanUICanvas(canvasId);

    const res = getShapeWithPointInsideFunc(canvasMousePosition[0], canvasMousePosition[1]);

    let canvasElemCtx = canvasElemObj.ctx_ui;
    drawline_m(canvasElemCtx, [[0, canvasMousePosition[1]], [canvasElemObj.c.width, canvasMousePosition[1]]], 'red');
    drawline_m(canvasElemCtx, [[canvasMousePosition[0], 0], [canvasMousePosition[0], canvasElemObj.c.height]], 'red');
    return res;
}

$("#clickandseeImageLeft_ui").click(function (e) {
    if (!module) {
        return;
    }

    if (g_leftSelected) {
        g_leftSelected = null;
        getShapeWithPointInsideCommon(e, "clickandseeImageLeft", getShapeWithPointInsideLeft);
    } else {
        g_leftSelected = getShapeWithPointInsideCommon(e, "clickandseeImageLeft", getShapeWithPointInsideLeft);
    }
});

$("#clickandseeImageRight_ui").click(function (e) {
    if (!module) {
        return;
    }
    if (g_rightSelected) {
        g_rightSelected = null;
        getShapeWithPointInsideCommon(e, "clickandseeImageRight", getShapeWithPointInsideRight);
    } else {
        g_rightSelected = getShapeWithPointInsideCommon(e, "clickandseeImageRight", getShapeWithPointInsideRight);
    }
});

function setAreaThresh() {
    const areaThresh = document.getElementById("areaThresh").value;
    g_areaThresh = parseInt(areaThresh);
    draw();
}

function setRatioVal() {
    const ratioSize = document.getElementById("cannyRatio").value;
    g_ratio = parseInt(ratioSize);
    draw();
}

function setBlurVal() {
    const blur_size = document.getElementById("cannyBlurSize").value;
    g_blurWidth = parseInt(blur_size);
    draw();
}

function setKernelVal() {
    let kernelSize = parseInt(document.getElementById("cannyKernelSize").value)+1;
    kernelSize += 1-(kernelSize%2);

    g_kernelSize = parseInt(kernelSize);
    draw();
}

function updateLookupCanvasHeap() {
    const lookupCanvas_ctx = document.getElementById('lookupCanvas_output').getContext("2d");
    const image = lookupCanvas_ctx.getImageData(0, 0,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height);

    Module.HEAP8.set(image.data, lookup_canvas_wasm_heap.ptr);
}

function updateDatabaseCanvasHeap() {
    const databaseCanvas_ctx = document.getElementById('databaseCanvas_output').getContext("2d");
    const image = databaseCanvas_ctx.getImageData(0, 0,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height);

    Module.HEAP8.set(image.data, canvas_inserted_in_database_wasm_heap.ptr);
}

async function addLayerToLookupCanvas(src) {
    await addLayer(g_transformState.interactiveCanvasState, src);
    draw();
    findMatches();
}

async function addLayerToDatabaseCanvas(src) {
    await addLayer(g_transformState.databaseCanvasState, src);
    g_flushCache = true;
    draw();
    findMatches();
}

async function addLayerToActiveCanvas(src) {

    await addLayer(g_transformState.activeCanvas, src);

    if (g_transformState.activeCanvas == g_transformState.databaseCanvasState) {
        g_flushCache = true;
    }

    draw();

    findMatches();
}

function drawShapeAndFragmentClickAndSee(imageHeap, width, height, shapeStr, shapeSize, canvasId, rotation) {
    const zoom = 1.0/1.5;

    var valHolder = new module.ValHolder(shapeSize*shapeSize*4);
    module.getImageFragmentFromShape(
        imageHeap, width, height,
        valHolder, shapeStr, shapeSize, zoom, rotation);

    const edgeImageOut5 = new ImageData(new Uint8ClampedArray(valHolder.outputImage2.val_),
        shapeSize, shapeSize);

    const ctxOutImage200 = getCleanCanvas(canvasId);

    ctxOutImage200.ctx.putImageData(edgeImageOut5, 0, 0);

    const bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom, rotation);
    const matrix = JSON.parse(bval)["mat"];
    const transshape = applyTransformationMatrixToAllPoints(shapeStrToShape(shapeStr), matrix);
    drawPolyFull(ctxOutImage200.ctx_ui, transshape, 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.1)');
    valHolder.delete();
}

function drawShapeAndFragment(imageHeap, width, height, shapeStr, shapeSize, canvasId, rotation) {
    const zoom = 1.0/1.5;

    var valHolder = new module.ValHolder(shapeSize*shapeSize*4);
    module.getImageFragmentFromShape(imageHeap, width, height, valHolder, shapeStr, shapeSize, zoom, rotation);

    const edgeImageOut5 = new ImageData(new Uint8ClampedArray(valHolder.outputImage2.val_),
        shapeSize, shapeSize);

    const ctxOutImage200 = getCleanCanvas(canvasId);

    ctxOutImage200.ctx.putImageData(edgeImageOut5, 0, 0);

    const bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom, rotation);
    const matrix = JSON.parse(bval)["mat"];

    const transshape = applyTransformationMatrixToAllPoints(shapeStrToShape(shapeStr), matrix);
    drawPolyFull(ctxOutImage200.ctx_ui, transshape, 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.2)');
    valHolder.delete();
}

function drawshapefromResult(shapeStr1, shapeStr2, hashDistance, rotation) {
    {
        const canctx = document.getElementById("lookupCanvas_uilower").getContext("2d");
        canctx.clearRect(0, 0, canctx.canvas.width, canctx.canvas.height);
        drawPolyFull(canctx, shapeStrToShape(shapeStr1), 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');
    }
    {
        const canctx = document.getElementById("databaseCanvas_uilower").getContext("2d");
        canctx.clearRect(0, 0, canctx.canvas.width, canctx.canvas.height);
        drawPolyFull(canctx, shapeStrToShape(shapeStr2), 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');
    }

    const hashDistSpan = document.getElementById("hammingDistanceForResult");
    hashDistSpan.innerHTML = "Hash distance: " + hashDistance;

    drawShapeAndFragment(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 256, "lookupCanvasFrag", 0);
    drawShapeAndFragment(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr2, 256, "databaseCanvasFrag", rotation);
}

let global_shapes = [];

function drawshapefromlist(index, shapeStr, rotation) {

    const can = getCleanUICanvas("lookupCanvas");
    drawPolyFull(can.ctx_ui, global_shapes[index]);

    const ctxEdge = getCleanUICanvas("canvasImgEdge");
    drawPolyFull(ctxEdge.ctx_ui, global_shapes[index]);

    const zoom = 1.0/1.5;

    let shapeSize = 400;

    var valHolder = new module.ValHolder(shapeSize*shapeSize*4);

    module.getImageFragmentFromShape(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        valHolder, shapeStr, shapeSize, zoom, rotation);
    const in3 = new Uint8ClampedArray(valHolder.outputImage2.val_);
    const edgeImageOut2 = new ImageData(in3, shapeSize, shapeSize);
    const ctxOutImage = getCleanCanvas("canvasImgFrag");
    ctxOutImage.ctx.putImageData(edgeImageOut2, 0, 0);
    let bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom, rotation);
    let matrix = JSON.parse(bval)["mat"];

    let transshape = applyTransformationMatrixToAllPoints(global_shapes[index], matrix);
    drawPolyFull(ctxOutImage.ctx_ui, transshape);


    shapeSize = 200;
    var valHolder2 = new module.ValHolder(shapeSize*shapeSize*4);
    module.getImageFragmentFromShape(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        valHolder2, shapeStr, shapeSize, zoom, rotation);

    const in5 = new Uint8ClampedArray(valHolder2.outputImage2.val_);
    const edgeImageOut5 = new ImageData(in5, shapeSize, shapeSize);
    const ctxOutImage200 = getCleanCanvas("canvasImgFrag200");
    ctxOutImage200.ctx.putImageData(edgeImageOut5, 0, 0);
    bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom, rotation);
    matrix = JSON.parse(bval)["mat"];

    transshape = applyTransformationMatrixToAllPoints(global_shapes[index], matrix);
    drawPolyFull(ctxOutImage200.ctx_ui, transshape);


    shapeSize = 32;
    var valHolder3 = new module.ValHolder(shapeSize*shapeSize*4);
    module.getImageFragmentFromShape(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        valHolder3, shapeStr, shapeSize, zoom, rotation);

    const in4 = new Uint8ClampedArray(valHolder3.outputImage2.val_);
    const edgeImageOut4 = new ImageData(in4, shapeSize, shapeSize);
    const ctxOutImage32 = getCleanCanvas("canvasImgFrag32");
    ctxOutImage32.ctx.putImageData(edgeImageOut4, 0, 0);
    bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom, rotation);
    matrix = JSON.parse(bval)["mat"];

    transshape = applyTransformationMatrixToAllPoints(global_shapes[index], matrix);
    drawPolyFull(ctxOutImage32.ctx_ui, transshape);

    valHolder.delete();
    valHolder2.delete();
    valHolder3.delete();
}

const randomColor = (() => {
    "use strict";

    const randomInt = (min, max) => {
        return Math.floor(Math.random() * (max - min + 1)) + min;
    };

    return () => {
        var h = randomInt(0, 360);
        var s = randomInt(42, 98);
        var l = randomInt(40, 90);
        return `${h},${s}%,${l}%`;
    };
})();

let g_matchesObj;

function drawMatches() {

}

function findMatches() {
    // let db = module.findMatchesForImageFromCanvas(
    //     canvas_inserted_in_database_wasm_heap.ptr,
    //     canvas_inserted_in_database_wasm_heap.width,
    //     canvas_inserted_in_database_wasm_heap.height,
    //     lookup_canvas_wasm_heap.ptr,
    //     lookup_canvas_wasm_heap.width,
    //     lookup_canvas_wasm_heap.height,
    //     360,
    //     g_thresh,
    //     g_ratio,
    //     g_kernelSize,
    //     g_blurWidth,
    //     g_areaThresh,
    //     g_flushCache
    // );
    // g_flushCache = false;
    //
    // let dbObj = JSON.parse(db);
    // g_matchesObj = dbObj;
    //
    // const list = document.getElementById('shapelist2');
    // list.innerHTML = "";
    // let keys = Object.keys(g_matchesObj);
    // for (let i = 0; i < keys.length; i++) {
    //     var key = keys[i];
    //     let opt = g_matchesObj[key];
    //     const color = "" + randomColor();
    //
    //     {
    //         let el = document.createElement("div");
    //         el.innerHTML = `<div class='shapeListEl' style="background-color: hsl(${color})" onmouseover="drawshapefromResult('${opt[1]}', '${opt[0]}', ${opt[2]}, ${opt[3]})" id='shapeListEl${i}'>${i}</div>`;
    //         list.appendChild(el);
    //     }
    //
    //     {
    //         const lookup = getCanvas("lookupCanvas");
    //         const database = getCanvas("databaseCanvas");
    //
    //         const stroke = 'hsl('+ color +')';
    //         const fill  = 'hsla('+ color +', 0.8)';
    //         drawPolyFull(lookup.ctx_ui,  shapeStrToShape(opt[1]), stroke, fill );
    //         drawPolyFull(database.ctx_ui,  shapeStrToShape(opt[0]), stroke, fill );
    //     }
    // }
    // drawMatches();
}

function shapeStrToShape(shapeStr) {
    return shapeStr.substring(9, shapeStr.length-2).split(',').map(x => x.split(' ').map(y => parseInt(y)));
}

function linesStrToLine(shapeStr) {
    return shapeStr.substring(11, shapeStr.length-1).split(',').map(x => x.split(' ').map(y => parseInt(y)));
}

function parseGlobalShapes(ctx, shapes) {
    var lines = shapes.split('\n');
    global_shapes = [];
    for(var i = 0;i < lines.length;i++){
        const vals = shapeStrToShape(lines[i]);
        // console.log(vals);
        let stroke1 = `rgba\(${Math.floor(Math.random() * 255)}, ${Math.floor(Math.random() * 255)}, ${Math.floor(Math.random() * 255)},`;
        drawPolyFull(ctx, vals, stroke1+" 1)", stroke1+" .3)");
        global_shapes.push(vals)
    }
    const list = document.getElementById('shapelist');
    list.innerHTML = "";
    for(let i = 0;i < lines.length;i++) {
        let opt = lines[i];
        let el = document.createElement("div");
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromlist(${i}, '${lines[i]}')" id='shapeListEl${i}'>${opt}</div>`;
        list.appendChild(el);
    }
}

function clearLowerUi() {
    {
        const ctx = document.getElementById("lookupCanvas_uilower").getContext("2d");
        ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
    }
    {
        const ctx = document.getElementById("databaseCanvas_uilower").getContext("2d");
        ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
    }
}

const enum_modes = {
    transform: 0,
    inspect: 1,
};

const enum_drawingImage = {
    RBGA: 0,
    valid_shapes: 1,
    all_shapes: 2,
    contours: 3,
    edge: 4,
};

function drawImageFromValHolder(width, height, valHolder, outputCanvasId, rgbId, imageName) {

    const ctx = document.getElementById(outputCanvasId).getContext("2d");

    switch (imageName) {
        case enum_drawingImage.RBGA:
            const canvas = document.getElementById(rgbId).getContext("2d").getImageData(0, 0, width, height);
            ctx.putImageData(canvas, 0, 0);
            break;
        case enum_drawingImage.valid_shapes:
            const outputImage3 = new ImageData(new Uint8ClampedArray(valHolder.outputImage3.val_), width, height);
            ctx.putImageData(outputImage3, 0, 0);
            break;
        case enum_drawingImage.all_shapes:
            const outputImage2 = new ImageData(new Uint8ClampedArray(valHolder.outputImage2.val_), width, height);
            ctx.putImageData(outputImage2, 0, 0);
            break;
        case enum_drawingImage.contours:
            const outputImage1 = new ImageData(new Uint8ClampedArray(valHolder.outputImage1.val_), width, height);
            ctx.putImageData(outputImage1, 0, 0);
            break;
        case enum_drawingImage.edge:
            const edgeImageOut = new ImageData(new Uint8ClampedArray(valHolder.edgeImage.val_), width, height);
            ctx.putImageData(edgeImageOut, 0, 0);
            break;
    }
}


function drawOutputImageOrEdgeImage() {
    //FIXME: we broke the other code
    // Wasm heap must already have the canvases loaded in memory

    clearLowerUi();

    {
        const width = lookup_canvas_wasm_heap.width;
        const height = lookup_canvas_wasm_heap.height;

        const valHolder = new module.ValHolder(lookup_canvas_wasm_heap.width*lookup_canvas_wasm_heap.height*4);

        module.encode(
            lookup_canvas_wasm_heap.ptr,
            lookup_canvas_wasm_heap.width,
            lookup_canvas_wasm_heap.height,
            valHolder, g_thresh, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);

        drawImageFromValHolder(width, height, valHolder, "lookupCanvas", "lookupCanvas_output",
            g_mainGlobalState.drawingImageLookup);

        drawImageFromValHolder(width, height, valHolder, "lookupCanvas2", "lookupCanvas_output",
            g_mainGlobalState.drawingImageLookup);

        valHolder.delete();
    }


    let res = module.transfromImage_keepVisable_wrapper(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        0.7,0.7,0,
        -0.7,0.7,0
    );
    // console.log(res.outStr);
    const shapeResults = JSON.parse(res.outStr);
    const edgeImageOut5 = new ImageData(new Uint8ClampedArray(res.v),
        res.width, res.height);

    console.log("shapeResults.invalid");
    console.log(shapeResults.invalid);

    setDatabaseCanvasSize(res.width, res.height);
    const databaseCanvas = getCleanCanvas("databaseCanvas");

    databaseCanvas.ctx.putImageData(edgeImageOut5, 0, 0);

    g_shapeResults = shapeResults;
    parseClickandseeShapesLeft(g_shapeResults);


    const list = document.getElementById('shapelist2');
    list.innerHTML = "";
    for (let i = 0; i < shapeResults.invalid.length; i++) {
        var obj = shapeResults.invalid[i];
        const color = "" + randomColor();

        {
            let el = document.createElement("div");
            // el.innerHTML = `<div class='shapeListEl' style="background-color: hsl(${color})" onmouseover="drawshapefromResult('${opt[1]}', '${opt[0]}', ${opt[2]}, ${opt[3]})" id='shapeListEl${i}'>${i}</div>`;

            el.innerHTML = `<div class='shapeListEl' style="background-color: hsl(${color})" onmouseover="drawshapefromClickandseeLeftRepeat('${obj.s1}','${obj.s2}')" id='shapeListEl___${i}'>${i} -- (${obj.dist})</div>`;
            list.appendChild(el);
        }
    }

    //FIXME: res.delete();

    // {
    //     const width = canvas_inserted_in_database_wasm_heap.width;
    //     const height = canvas_inserted_in_database_wasm_heap.height;
    //
    //     const valHolder = new module.ValHolder(canvas_inserted_in_database_wasm_heap.width*canvas_inserted_in_database_wasm_heap.height*4);
    //
    //     module.encode(
    //         canvas_inserted_in_database_wasm_heap.ptr,
    //         canvas_inserted_in_database_wasm_heap.width,
    //         canvas_inserted_in_database_wasm_heap.height,
    //         valHolder, g_thresh, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);
    //     let jsonData = module.getContoursWithCurvature(lookup_canvas_wasm_heap.ptr,
    //         lookup_canvas_wasm_heap.width,
    //         lookup_canvas_wasm_heap.height,
    //         g_thresh,
    //         g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);
    //     console.log(JSON.parse(jsonData));
    //     drawImageFromValHolder(width, height, valHolder, "databaseCanvas", "databaseCanvas_output",
    //         g_mainGlobalState.drawingImageDatabase);
    //
    //     valHolder.delete();
    // }

}

function setMode(mode) {
    g_mainGlobalState.mode = mode;

    const rotationSliderWrapper = document.getElementById("rotationSliderWrapper");
    const clickandseeListLeft = document.getElementById("clickandseeListLeft");
    const clickandseeListRight = document.getElementById("clickandseeListRight");
    if (g_mainGlobalState.mode == enum_modes.inspect) {
        g_rightSelected = null;
        g_leftSelected = null;
        rotationSliderWrapper.style.display = "block";
        clickandseeListLeft.innerHTML = "";
        clickandseeListRight.innerHTML = "";
    } else {
        g_rightSelected = null;
        g_leftSelected = null;
        rotationSliderWrapper.style.display = "none";
        clickandseeListLeft.innerHTML = "";
        clickandseeListRight.innerHTML = "";
    }

    draw();
}

function main() {
    g_thresh = module.get_CANNY_THRESH();
    g_ratio = module.get_CANNY_RATIO();
    g_kernelSize = module.get_CANNY_KERNEL_SIZE();
    g_blurWidth = module.get_CANNY_BLUR_WIDTH();
    g_areaThresh = module.get_CANNY_AREA_THRESH();

    document.getElementById("cannyBlurSize").value = g_blurWidth;
    document.getElementById("cannyKernelSize").value = g_kernelSize;
    document.getElementById("cannyRatio").value = g_ratio;
    document.getElementById("areaThresh").value = g_areaThresh;

    init_loadTransformStateAndImages();

    g_mainGlobalState = {};
    g_mainGlobalState.transformState = g_transformState;
    g_mainGlobalState.drawingImageLookup = enum_drawingImage.RBGA;
    g_mainGlobalState.drawingImageDatabase = enum_drawingImage.RBGA;
    g_mainGlobalState.mode = enum_modes.transform;
}

