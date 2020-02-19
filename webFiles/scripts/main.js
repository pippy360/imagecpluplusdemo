async function loadImage(src) {
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
    let canvas2 = document.getElementById('shapeDemo2');
    let ctx2 = canvas2.getContext('2d');
    ctx2.drawImage(img, 0, 0, ctx2.canvas.width, ctx2.canvas.height);

    let canvas3 = document.getElementById('shapeDemoResult2');
    let ctx3 = canvas3.getContext('2d');
    ctx3.drawImage(img, 0, 0, ctx2.canvas.width, ctx2.canvas.height);

    const image = ctx2.getImageData(0, 0, canvas2.width, canvas2.height);
    Module.HEAP8.set(image.data, heap_image_og);

    draw();
    copyimagetocpp();

}

function drawShapeAndFragment(imageHeap, shapeStr, shapeSize, canvasId) {
    const zoom = 1.0/1.5;

    var valHolder = new module.ValHolder(shapeSize*shapeSize*4);
    module.getHashesForShape2(imageHeap, valHolder, shapeStr, shapeSize, zoom);

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

    const can = getCleanUICanvas("shapeDemoResult");
    drawPolyFull(can.ctx_ui, shapeStrToShape(shapeStr1));

    const ctxEdge = getCleanUICanvas("shapeDemoResult2");
    drawPolyFull(ctxEdge.ctx_ui, shapeStrToShape(shapeStr2));

    const zoom = 1.0/1.5;

    drawShapeAndFragment(heap_image_in, shapeStr1, 400, "canvasImgFrag2");
    drawShapeAndFragment(heap_image_og, shapeStr2, 400, "canvasImgFrag2Right");

    drawShapeAndFragment(heap_image_in, shapeStr1, 200, "canvasImgFrag2002");
    drawShapeAndFragment(heap_image_og, shapeStr2, 200, "canvasImgFrag2002Right");

    drawShapeAndFragment(heap_image_in, shapeStr1, 32, "canvasImgFrag322");
    drawShapeAndFragment(heap_image_og, shapeStr2, 32, "canvasImgFrag322Right");
}

let global_shapes = [];

function drawshapefromlist(index, shapeStr) {

    const can = getCleanUICanvas("shapeDemo");
    drawPolyFull(can.ctx_ui, global_shapes[index]);

    const ctxEdge = getCleanUICanvas("canvasImgEdge");
    drawPolyFull(ctxEdge.ctx_ui, global_shapes[index]);

    const zoom = 1.0/1.5;

    let shapeSize = 400;
    module.getHashesForShape2(heap_image_in, g_valHolder, shapeStr, shapeSize, zoom);
    const in3 = new Uint8ClampedArray(g_valHolder.outputImage2.val_);
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
    module.getHashesForShape2(heap_image_in, valHolder2, shapeStr, shapeSize, zoom);

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
    module.getHashesForShape2(heap_image_in, valHolder3, shapeStr, shapeSize, zoom);

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

    valHolder2.delete();
    valHolder3.delete();
}

function findMatches() {
    var db = module.getAllTheHashesForImageFromCanvas(heap_image_og, 360);
    var check = module.getAllTheHashesForImageFromCanvas(heap_image_in, 360);

    let dbObj = JSON.parse(db);
    let checkObj = JSON.parse(check);

    console.log(dbObj);
    console.log(checkObj);

    const list = document.getElementById('shapelist2');
    list.innerHTML = "";
    let keys = Object.keys(dbObj);
    for (var i = 0; i < keys.length; i++)
    {
        var key = keys[i];
        if (checkObj[key] !== undefined) {
            let opt = checkObj[key];
            let opt2 = dbObj[key];
            let el = document.createElement("div");
            el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromResult('${opt}', '${opt2}')" id='shapeListEl${i}'>${opt}</div>`;
            list.appendChild(el);
        }
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

function copyimagetocpp() {
    // const c_shapeDemo = getCanvas("shapeDemo");
    //
    // // Load image
    // // Make canvas same size as image
    const canvas = document.getElementById('shapeDemo');
    const ctx = canvas.getContext('2d');
    const image = ctx.getImageData(0, 0, canvas.width, canvas.height);

    Module.HEAP8.set(image.data, heap_image_in);
    let kernelSize = parseInt(document.getElementById("cannyKernelSize").value)+1;
    kernelSize += 1-(kernelSize%2);
    const ratioSize = document.getElementById("cannyRatio").value;

    console.log("kernelSize" + kernelSize);
    console.log("ratioSize" + ratioSize);

    module.encode(heap_image_in, g_valHolder, canvas.width, canvas.height, 100, parseInt(ratioSize), parseInt(kernelSize), 6);
    const ctx2 = document.getElementById('outputImageCanvas').getContext('2d');
    const ctxEdge = getCleanCanvas("canvasImgEdge");

    const in1 = new Uint8ClampedArray(g_valHolder.outputImage1.val_);
    const in2 = new Uint8ClampedArray(g_valHolder.edgeImage.val_);
    const imageout = new ImageData(in1, canvas.width, canvas.height);
    const edgeImageOut = new ImageData(in2, canvas.width, canvas.height);

    ctx2.putImageData(imageout, 0, 0);
    ctxEdge.ctx.putImageData(edgeImageOut, 0, 0);



    const shapeDemo_ui = document.getElementById('shapeDemo_ui').getContext('2d');
    parseGlobalShapes(shapeDemo_ui, g_valHolder.shapeStr);
    setTimeout(findMatches(), 0);
}

function main() {
    loadImage(g_img.src);
    copyimagetocpp();
}

