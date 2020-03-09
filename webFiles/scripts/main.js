let g_fragmentZoom = 1.0/1.5;
let g_blurWidth = 3;
let g_kernelSize = 3;
let g_ratio = 3;
let g_thresh = 100;
let g_areaThresh = 200;
let g_flushCache = true;

function drawshapefromClickandseeLeft(shapeStr1) {

    const can = getCleanUICanvas("clickandseeImageLeft");
    drawPolyFull(can.ctx_ui, shapeStrToShape(shapeStr1));

    drawShapeAndFragmentClickAndSee(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr1, 400, "clickandseeFragLeft");

    drawShapeAndFragmentClickAndSee(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr1, 200, "clickandseeFragLeft");

    drawShapeAndFragmentClickAndSee(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr1, 32, "clickandseeFragLeft");
}

function drawshapefromClickandseeRight(shapeStr1) {

    const can = getCleanUICanvas("clickandseeImageRight");
    drawPolyFull(can.ctx_ui, shapeStrToShape(shapeStr1));

    drawShapeAndFragmentClickAndSee(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 400, "clickandseeFragRight");

    drawShapeAndFragmentClickAndSee(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 200, "clickandseeFragRight");

    drawShapeAndFragmentClickAndSee(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 32, "clickandseeFragRight");
}

function parseClickandseeShapesLeft(shapes) {
    var lines = shapes.split('\n');
    const list = document.getElementById("clickandseeListLeft");
    list.innerHTML = "";
    for (var i = 0;i < lines.length;i++){

        if (lines[i] == "")
            continue;

        let opt = lines[i];
        let el = document.createElement("div");
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromClickandseeLeft('${opt}')" id='clickandseeShapeListElm${i}'>-----${opt}</div>`;
        list.appendChild(el);
    }
}

function parseClickandseeShapesRight(shapes) {
    var lines = shapes.split('\n');
    const list = document.getElementById("clickandseeListRight");
    list.innerHTML = "";
    for (var i = 0;i < lines.length;i++){

        if (lines[i] == "")
            continue;

        let opt = lines[i];
        let el = document.createElement("div");
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromClickandseeRight('${opt}')" id='clickandseeShapeListElm${i}'>-----${opt}</div>`;
        list.appendChild(el);
    }
}

function getShapeWithPointInsideLeft(x, y) {
    const shapeStr = module.getShapeWithPointInside(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        x, y, 100, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);

    parseClickandseeShapesLeft(shapeStr);
}

function getShapeWithPointInsideRight(x, y) {
    const shapeStr = module.getShapeWithPointInside(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        x, y, 100, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);

    parseClickandseeShapesRight(shapeStr);
}

$("#clickandseeImageLeft_ui").mousedown(function (e) {
    var canvasElem = $("#clickandseeImageLeft_ui")[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    getShapeWithPointInsideLeft(canvasMousePosition[0], canvasMousePosition[1]);

    let canvasElemObj = getCleanUICanvas("clickandseeImageLeft");
    let canvasElemCtx = canvasElemObj.ctx_ui;
    drawline_m(canvasElemCtx, [[0, canvasMousePosition[1]], [400, canvasMousePosition[1]]], 'red');
    drawline_m(canvasElemCtx, [[canvasMousePosition[0], 0], [canvasMousePosition[0], 400]], 'red');
});

$("#clickandseeImageRight_ui").mousedown(function (e) {
    var canvasElem = $("#clickandseeImageRight_ui")[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    getShapeWithPointInsideRight(canvasMousePosition[0], canvasMousePosition[1]);

    let canvasElemObj = getCleanUICanvas("clickandseeImageRight");
    let canvasElemCtx = canvasElemObj.ctx_ui;
    drawline_m(canvasElemCtx, [[0, canvasMousePosition[1]], [400, canvasMousePosition[1]]], 'red');
    drawline_m(canvasElemCtx, [[canvasMousePosition[0], 0], [canvasMousePosition[0], 400]], 'red');
});

function setAreaThresh() {
    const areaThresh = document.getElementById("areaThresh").value;
    g_areaThresh = parseInt(areaThresh);
}

function setRatioVal() {
    const ratioSize = document.getElementById("cannyRatio").value;
    g_ratio = parseInt(ratioSize);
}

function setBlurVal() {
    const blur_size = document.getElementById("cannyBlurSize").value;
    g_blurWidth = parseInt(blur_size);
}

function setKernelVal() {
    let kernelSize = parseInt(document.getElementById("cannyKernelSize").value)+1;
    kernelSize += 1-(kernelSize%2);

    g_kernelSize = parseInt(kernelSize);
}

function updateLookupCanvasHeap() {
    const lookupCanvas = getCanvas('lookupCanvas');
    const image = lookupCanvas.ctx.getImageData(0, 0,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height);

    Module.HEAP8.set(image.data, lookup_canvas_wasm_heap.ptr);
}

function updateDatabaseCanvasHeap() {
    const databaseCanvas = getCanvas('databaseCanvas');
    const image = databaseCanvas.ctx.getImageData(0, 0,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height);

    Module.HEAP8.set(image.data, canvas_inserted_in_database_wasm_heap.ptr);
}

async function loadImage(src) {
    g_flushCache = true;
    g_img.src = src;
    // Load image
    const imgBlob = await fetch(src).then(resp => resp.blob());
    const img = await createImageBitmap(imgBlob);
    // Make canvas same size as image
    const canvas = document.getElementById('canvasImg');
    canvas.width = img.width;
    canvas.height = img.height;
    // Draw image onto canvas
    const ctx = canvas.getContext('2d');
    ctx.drawImage(img, 0, 0);

    //const image = ctx.getImageData(0, 0, img.width, img.height);
    const databaseCanvas = getCleanCanvas('databaseCanvas');
    databaseCanvas.ctx.drawImage(img, 0, 0);

    const lookupCanvas = getCleanCanvas('lookupCanvas');
    lookupCanvas.ctx.drawImage(img, 0, 0);

    updateLookupCanvasHeap();
    updateDatabaseCanvasHeap();

    draw();
    // loadEdgeImages();
}

function drawShapeAndFragmentClickAndSee(imageHeap, width, height, shapeStr, shapeSize, canvasId) {
    const zoom = 1.0/1.5;

    var valHolder = new module.ValHolder(shapeSize*shapeSize*4);
    module.getImageFragmentFromShape(
        imageHeap, width, height,
        valHolder, shapeStr, shapeSize, zoom);

    const edgeImageOut5 = new ImageData(new Uint8ClampedArray(valHolder.outputImage2.val_),
        shapeSize, shapeSize);

    const ctxOutImage200 = getCleanCanvas(canvasId + shapeSize);

    ctxOutImage200.ctx.putImageData(edgeImageOut5, 0, 0);

    const bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom);
    const matrix = [
        [bval[0], bval[1], bval[2]],
        [bval[3], bval[4], bval[5]],
        [bval[6], bval[7], bval[8]],
    ];
    const transshape = applyTransformationMatrixToAllPoints(shapeStrToShape(shapeStr), matrix);
    drawPolyFull(ctxOutImage200.ctx_ui, transshape);
    valHolder.delete();
}

function drawShapeAndFragment(imageHeap, width, height, shapeStr, shapeSize, canvasId) {
    const zoom = 1.0/1.5;

    var valHolder = new module.ValHolder(shapeSize*shapeSize*4);
    module.getImageFragmentFromShape(imageHeap, width, height, valHolder, shapeStr, shapeSize, zoom);

    const edgeImageOut5 = new ImageData(new Uint8ClampedArray(valHolder.outputImage2.val_),
            shapeSize, shapeSize);

    const ctxOutImage200 = getCleanCanvas(canvasId);

    ctxOutImage200.ctx.putImageData(edgeImageOut5, 0, 0);

    const bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom);
    const matrix = [
        [bval[0], bval[1], bval[2]],
        [bval[3], bval[4], bval[5]],
        [bval[6], bval[7], bval[8]],
    ];
    const transshape = applyTransformationMatrixToAllPoints(shapeStrToShape(shapeStr), matrix);
    drawPolyFull(ctxOutImage200.ctx_ui, transshape);
    valHolder.delete();
}

function drawshapefromResult(shapeStr1, shapeStr2) {

    const can = getCleanUICanvas("lookupCanvas");
    drawPolyFull(can.ctx_ui, shapeStrToShape(shapeStr1));

    const ctxEdge = getCleanUICanvas("databaseCanvas");
    drawPolyFull(ctxEdge.ctx_ui, shapeStrToShape(shapeStr2));

    const zoom = 1.0/1.5;

    drawShapeAndFragment(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 400, "canvasImgFrag2");
    drawShapeAndFragment(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr2, 400, "canvasImgFrag2Right");

    drawShapeAndFragment(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 200, "canvasImgFrag2002");
    drawShapeAndFragment(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr2, 200, "canvasImgFrag2002Right");

    drawShapeAndFragment(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 32, "canvasImgFrag322");
    drawShapeAndFragment(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr2, 32, "canvasImgFrag322Right");
}

let global_shapes = [];

function drawshapefromlist(index, shapeStr) {

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
        valHolder, shapeStr, shapeSize, zoom);
    const in3 = new Uint8ClampedArray(valHolder.outputImage2.val_);
    const edgeImageOut2 = new ImageData(in3, shapeSize, shapeSize);
    const ctxOutImage = getCleanCanvas("canvasImgFrag");
    ctxOutImage.ctx.putImageData(edgeImageOut2, 0, 0);
    let bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom);
    let matrix = [
        [bval[0], bval[1], bval[2]],
        [bval[3], bval[4], bval[5]],
        [bval[6], bval[7], bval[8]],
    ];
    let transshape = applyTransformationMatrixToAllPoints(global_shapes[index], matrix);
    drawPolyFull(ctxOutImage.ctx_ui, transshape);


    shapeSize = 200;
    var valHolder2 = new module.ValHolder(shapeSize*shapeSize*4);
    module.getImageFragmentFromShape(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        valHolder2, shapeStr, shapeSize, zoom);

    const in5 = new Uint8ClampedArray(valHolder2.outputImage2.val_);
    const edgeImageOut5 = new ImageData(in5, shapeSize, shapeSize);
    const ctxOutImage200 = getCleanCanvas("canvasImgFrag200");
    ctxOutImage200.ctx.putImageData(edgeImageOut5, 0, 0);
    bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom);
    matrix = [
        [bval[0], bval[1], bval[2]],
        [bval[3], bval[4], bval[5]],
        [bval[6], bval[7], bval[8]],
    ];
    transshape = applyTransformationMatrixToAllPoints(global_shapes[index], matrix);
    drawPolyFull(ctxOutImage200.ctx_ui, transshape);


    shapeSize = 32;
    var valHolder3 = new module.ValHolder(shapeSize*shapeSize*4);
    module.getImageFragmentFromShape(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        valHolder3, shapeStr, shapeSize, zoom);

    const in4 = new Uint8ClampedArray(valHolder3.outputImage2.val_);
    const edgeImageOut4 = new ImageData(in4, shapeSize, shapeSize);
    const ctxOutImage32 = getCleanCanvas("canvasImgFrag32");
    ctxOutImage32.ctx.putImageData(edgeImageOut4, 0, 0);
    bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom);
    matrix = [
        [bval[0], bval[1], bval[2]],
        [bval[3], bval[4], bval[5]],
        [bval[6], bval[7], bval[8]],
    ];
    transshape = applyTransformationMatrixToAllPoints(global_shapes[index], matrix);
    drawPolyFull(ctxOutImage32.ctx_ui, transshape);

    valHolder.delete();
    valHolder2.delete();
    valHolder3.delete();
}

function findMatches() {
//std::string findMatchesForImageFromCanvas(uintptr_t img_in, uintptr_t img_in2, int rotation,
//     int thresh,
//     int ratio,
//     int kernel_size,
//     int blur_width)

    var db = module.findMatchesForImageFromCanvas(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        360,
        g_thresh,
        g_ratio,
        g_kernelSize,
        g_blurWidth,
        g_flushCache
    );
    // g_flushCache = false;//FIXME: we should cache the database on

    let dbObj = JSON.parse(db);

    console.log(dbObj);

    const list = document.getElementById('shapelist2');
    list.innerHTML = "";
    let keys = Object.keys(dbObj);
    for (var i = 0; i < keys.length; i++)
    {
        var key = keys[i];
        let opt = dbObj[key];
        let el = document.createElement("div");
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromResult('${opt[1]}', '${opt[0]}')" id='shapeListEl${i}'>${i}</div>`;
        list.appendChild(el);
    }
}

function shapeStrToShape(shapeStr) {
    return shapeStr.substring(9, shapeStr.length-2).split(',').map(x => x.split(' ').map(y => parseInt(y)));
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

//FIXME: rename
function loadEdgeImages() {

    // Wasm heap must already have the canvases loaded in memory

    const lookupCanvas = getCanvas('lookupCanvas');
    const width = lookup_canvas_wasm_heap.width;
    const height = lookup_canvas_wasm_heap.height;

    var valHolder = new module.ValHolder(lookup_canvas_wasm_heap.width*lookup_canvas_wasm_heap.height*4);

    module.encode(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        valHolder, 100, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);

    const outputImage3 = new ImageData(new Uint8ClampedArray(valHolder.outputImage3.val_), width, height);
    const outputImage2 = new ImageData(new Uint8ClampedArray(valHolder.outputImage2.val_), width, height);
    const outputImage1 = new ImageData(new Uint8ClampedArray(valHolder.outputImage1.val_), width, height);
    const edgeImageOut = new ImageData(new Uint8ClampedArray(valHolder.edgeImage.val_), width, height);

    getCleanCanvas("canvasImgEdgeHullValidLeft").ctx.putImageData(outputImage3, 0, 0);
    getCleanCanvas("bluredGreyOutputImage").ctx.putImageData(outputImage1, 0, 0);
    getCleanCanvas("canvasImgEdge").ctx.putImageData(edgeImageOut, 0, 0);
    getCleanCanvas("canvasImgEdgeContoursLeft").ctx.putImageData(outputImage2, 0, 0);

    parseGlobalShapes(lookupCanvas.ctx_ui, valHolder.shapeStr);
    setTimeout(findMatches(), 0);

    valHolder.delete();
}

function main() {
    loadImage(g_img.src);
    // loadEdgeImages();
}

