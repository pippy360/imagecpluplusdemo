let g_fragmentZoom = 1.0/1.5;
let g_blurWidth = 3;
let g_kernelSize = 3;
let g_ratio = 3;
let g_thresh = 100;
let g_areaThresh = 200;
let g_flushCache = true;

let g_leftSelected;
let g_rightSelected;

function drawshapefromClickandseeLeft(shapeStr1) {
    console.log("drawshapefromClickandseeLeft called with: "+shapeStr1);
    drawShapeAndFragmentClickAndSee(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr1, 256, "clickandseeFragLeft");

    drawShapeAndFragmentClickAndSee(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr1, 128, "clickandseeFragLeft");

    drawShapeAndFragmentClickAndSee(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr1, 32, "clickandseeFragLeft");
}

function drawshapefromClickandseeRight(shapeStr1) {

    drawShapeAndFragmentClickAndSee(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 256, "clickandseeFragRight");

    drawShapeAndFragmentClickAndSee(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 128, "clickandseeFragRight");

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

    const can = getCleanUICanvas("clickandseeImageLeft");
    for (let i = 0;i < lines.length;i++){

        if (lines[i] == "")
            continue;

        let opt = lines[i];
        let el = document.createElement("div");
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromClickandseeLeft('${opt}')" id='clickandseeShapeListElm${i}'>-----${i}</div>`;
        list.appendChild(el);

        drawPolyFull(can.ctx_ui, shapeStrToShape(opt), 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');
    }
}

function parseClickandseeShapesRight(shapes) {
    var lines = shapes.split('\n');
    const list = document.getElementById("clickandseeListRight");
    list.innerHTML = "";

    const can = getCleanUICanvas("clickandseeImageRight");
    for (let i = 0;i < lines.length;i++){

        if (lines[i] == "")
            continue;

        let opt = lines[i];
        let el = document.createElement("div");
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromClickandseeRight('${opt}')" id='clickandseeShapeListElm${i}'>-----${i}</div>`;
        list.appendChild(el);

        drawPolyFull(can.ctx_ui, shapeStrToShape(opt), 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.6)');
    }
}

function getShapeWithPointInsideLeft(x, y) {
    const shapeStr = module.getShapeWithPointInside(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        x, y, 100, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);

    parseClickandseeShapesLeft(shapeStr);

    return shapeStr;
}

function getShapeWithPointInsideRight(x, y) {
    const shapeStr = module.getShapeWithPointInside(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        x, y, 100, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);

    parseClickandseeShapesRight(shapeStr);

    return shapeStr;
}



$("#clickandseeImageLeft_ui").mousemove(function (e) {
    if (!module || g_leftSelected) {
        return;
    }
    var canvasElem = $("#clickandseeImageLeft_ui")[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    let canvasElemObj = getCleanUICanvas("clickandseeImageLeft");

    getShapeWithPointInsideLeft(canvasMousePosition[0], canvasMousePosition[1]);

    let canvasElemCtx = canvasElemObj.ctx_ui;
    drawline_m(canvasElemCtx, [[0, canvasMousePosition[1]], [canvasElemObj.c.width, canvasMousePosition[1]]], 'red');
    drawline_m(canvasElemCtx, [[canvasMousePosition[0], 0], [canvasMousePosition[0], canvasElemObj.c.height]], 'red');
});

$("#clickandseeImageLeft_ui").click(function (e) {
    if (!module) {
        return;
    }
    var canvasElem = $("#clickandseeImageLeft_ui")[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    let canvasElemObj = getCleanUICanvas("clickandseeImageLeft");

    g_leftSelected = getShapeWithPointInsideLeft(canvasMousePosition[0], canvasMousePosition[1]);

    let canvasElemCtx = canvasElemObj.ctx_ui;
    drawline_m(canvasElemCtx, [[0, canvasMousePosition[1]], [canvasElemObj.c.width, canvasMousePosition[1]]], 'red');
    drawline_m(canvasElemCtx, [[canvasMousePosition[0], 0], [canvasMousePosition[0], canvasElemObj.c.height]], 'red');
});

$("#clickandseeImageRight_ui").mousemove(function (e) {
    if (!module || g_rightSelected) {
        return;
    }

    var canvasElem = $("#clickandseeImageRight_ui")[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    let canvasElemObj = getCleanUICanvas("clickandseeImageRight");

    getShapeWithPointInsideRight(canvasMousePosition[0], canvasMousePosition[1]);

    let canvasElemCtx = canvasElemObj.ctx_ui;
    drawline_m(canvasElemCtx, [[0, canvasMousePosition[1]], [canvasElemObj.c.width, canvasMousePosition[1]]], 'red');
    drawline_m(canvasElemCtx, [[canvasMousePosition[0], 0], [canvasMousePosition[0], canvasElemObj.c.height]], 'red');
});

$("#clickandseeImageRight_ui").click(function (e) {
    if (!module) {
        return;
    }

    var canvasElem = $("#clickandseeImageRight_ui")[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    let canvasElemObj = getCleanUICanvas("clickandseeImageRight");

    g_rightSelected = getShapeWithPointInsideRight(canvasMousePosition[0], canvasMousePosition[1]);

    let canvasElemCtx = canvasElemObj.ctx_ui;
    drawline_m(canvasElemCtx, [[0, canvasMousePosition[1]], [canvasElemObj.c.width, canvasMousePosition[1]]], 'red');
    drawline_m(canvasElemCtx, [[canvasMousePosition[0], 0], [canvasMousePosition[0], canvasElemObj.c.height]], 'red');
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
    console.log("Loading image: "+src);
    g_flushCache = true;
    g_img.onload = function () {
        draw();

        updateLookupCanvasHeap();
        updateDatabaseCanvasHeap();

        loadEdgeImages();
        findMatches();
    };
    g_img.src = src;
    g_rightSelected = null;
    g_leftSelected = null;
}

function drawShapeAndFragmentClickAndSee(imageHeap, width, height, shapeStr, shapeSize, canvasId) {
    const zoom = 1.0/1.5;

    var valHolder = new module.ValHolder(shapeSize*shapeSize*4);
    module.getImageFragmentFromShape(
        imageHeap, width, height,
        valHolder, shapeStr, shapeSize, zoom);

    const edgeImageOut5 = new ImageData(new Uint8ClampedArray(valHolder.outputImage2.val_),
        shapeSize, shapeSize);

    console.log("clean: " + canvasId + shapeSize );
    const ctxOutImage200 = getCleanCanvas(canvasId + shapeSize);

    ctxOutImage200.ctx.putImageData(edgeImageOut5, 0, 0);

    const bval = module.calcMatrixFromString(shapeStr, shapeSize, zoom);
    const matrix = JSON.parse(bval)["mat"];
    const transshape = applyTransformationMatrixToAllPoints(shapeStrToShape(shapeStr), matrix);
    console.log("draw" + transshape);
    drawPolyFull(ctxOutImage200.ctx_ui, transshape, 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.1)');
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
    const matrix = JSON.parse(bval)["mat"];

    const transshape = applyTransformationMatrixToAllPoints(shapeStrToShape(shapeStr), matrix);
    drawPolyFull(ctxOutImage200.ctx_ui, transshape, 'rgb(45, 0, 255)', 'rgba(45, 0, 255, 0.2)');
    valHolder.delete();
}

function drawshapefromResult(shapeStr1, shapeStr2) {
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

    drawShapeAndFragment(
        lookup_canvas_wasm_heap.ptr,
        lookup_canvas_wasm_heap.width,
        lookup_canvas_wasm_heap.height,
        shapeStr1, 256, "lookupCanvasFrag");
    drawShapeAndFragment(
        canvas_inserted_in_database_wasm_heap.ptr,
        canvas_inserted_in_database_wasm_heap.width,
        canvas_inserted_in_database_wasm_heap.height,
        shapeStr2, 256, "databaseCanvasFrag");
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
    let matrix = JSON.parse(bval)["mat"];

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
    matrix = JSON.parse(bval)["mat"];

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
    g_flushCache = false;

    let dbObj = JSON.parse(db);
    g_matchesObj = dbObj;

    const list = document.getElementById('shapelist2');
    list.innerHTML = "";
    let keys = Object.keys(g_matchesObj);
    for (var i = 0; i < keys.length; i++) {
        var key = keys[i];
        let opt = g_matchesObj[key];
        const color = "" + randomColor();

        {
            let el = document.createElement("div");
            el.innerHTML = `<div class='shapeListEl' style="background-color: hsl(${color})" onmouseover="drawshapefromResult('${opt[1]}', '${opt[0]}')" id='shapeListEl${i}'>${i}</div>`;
            list.appendChild(el);
        }

        {
            const lookup = getCanvas("lookupCanvas");
            const database = getCanvas("databaseCanvas");

            const stroke = 'hsl('+ color +')';
            const fill  = 'hsla('+ color +', 0.8)';
            drawPolyFull(lookup.ctx_ui,  shapeStrToShape(opt[1]), stroke, fill );
            drawPolyFull(database.ctx_ui,  shapeStrToShape(opt[0]), stroke, fill );
        }
    }
    drawMatches();
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

    {
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
        getCleanCanvas("canvasImgEdgeContoursLeft").ctx.putImageData(outputImage2, 0, 0);
        getCleanCanvas("contoursImageLeft").ctx.putImageData(outputImage1, 0, 0);
        getCleanCanvas("canvasEdgeImageLeft").ctx.putImageData(edgeImageOut, 0, 0);

        valHolder.delete();
    }

    {
        const width = canvas_inserted_in_database_wasm_heap.width;
        const height = canvas_inserted_in_database_wasm_heap.height;

        var valHolder = new module.ValHolder(canvas_inserted_in_database_wasm_heap.width*canvas_inserted_in_database_wasm_heap.height*4);

        module.encode(
            canvas_inserted_in_database_wasm_heap.ptr,
            canvas_inserted_in_database_wasm_heap.width,
            canvas_inserted_in_database_wasm_heap.height,
            valHolder, 100, g_ratio, g_kernelSize, g_blurWidth, g_areaThresh);

        const outputImage3 = new ImageData(new Uint8ClampedArray(valHolder.outputImage3.val_), width, height);
        const outputImage2 = new ImageData(new Uint8ClampedArray(valHolder.outputImage2.val_), width, height);
        const outputImage1 = new ImageData(new Uint8ClampedArray(valHolder.outputImage1.val_), width, height);
        const edgeImageOut = new ImageData(new Uint8ClampedArray(valHolder.edgeImage.val_), width, height);

        getCleanCanvas("canvasImgEdgeHullValidRight").ctx.putImageData(outputImage3, 0, 0);
        getCleanCanvas("canvasImgEdgeContoursRight").ctx.putImageData(outputImage2, 0, 0);
        getCleanCanvas("contoursImageRight").ctx.putImageData(outputImage1, 0, 0);
        getCleanCanvas("canvasEdgeImageRight").ctx.putImageData(edgeImageOut, 0, 0);

        valHolder.delete();
    }

}

function main() {
    loadImage(g_img.src);

    $( "#databaseAndLookupCanvasWrapper" ).hover(
        function() {
            // const lookup = getCleanUICanvas("lookupCanvas");
            // const database = getCleanUICanvas("databaseCanvas");

            // drawMatches();

            // lookup.c.style.opacity = "0.5";
            // database.c.style.opacity = "0.5";
        }, function() {
            // const lookup = getCleanUICanvas("lookupCanvas");
            // const database = getCleanUICanvas("databaseCanvas");
            //
            // lookup.c.style.opacity = "1";
            // database.c.style.opacity = "1";
        }
    );
}

