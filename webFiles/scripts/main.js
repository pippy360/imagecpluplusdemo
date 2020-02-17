async function loadImage(src) {
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
    let canvas2 = document.getElementById('shapeDemo');
    let ctx2 = canvas2.getContext('2d');
    ctx2.drawImage(img, 0, 0, ctx2.canvas.width, ctx2.canvas.height);
    const image = ctx2.getImageData(0, 0, canvas2.width, canvas2.height);
    Module.HEAP8.set(image.data, heap_image_og);
}

let global_shapes = [];

function drawshapefromlist(index, shapeStr) {

    const can = getCleanUICanvas("shapeDemo");
    const ctxEdge = getCleanUICanvas("canvasImgEdge");
    drawPolyFull(can.ctx_ui, global_shapes[index]);
    drawPolyFull(ctxEdge.ctx_ui, global_shapes[index]);

    const zoom = 1.0/1.5;

    let shapeSize = 400;
    module.getHashesForShape2(heap_image_in, valHolder, shapeStr, shapeSize, zoom);
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
    var db = module.getAllTheHashesForImageFromCanvas(heap_image_og, 90);
    var check = module.getAllTheHashesForImageFromCanvas(heap_image_in, 90);

    let dbObj = JSON.parse(db);
    let checkObj = JSON.parse(check);

    console.log(dbObj);
    console.log(checkObj);

    let keys = Object.keys(dbObj);
    for (var i = 0; i < keys.length; i++)
    {
        var key = keys[i];
        if (checkObj[key] !== undefined) {
            console.log("match");
        }
    }
}

function parseGlobalShapes(ctx, shapes) {
    var lines = shapes.split('\n');
    global_shapes = []
    for(var i = 0;i < lines.length;i++){
        const vals = lines[i].substring(9, lines[i].length-2).split(',').map(x => x.split(' ').map(y => parseInt(y)));
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

    module.encode(heap_image_in, valHolder, canvas.width, canvas.height, 100, parseInt(ratioSize), parseInt(kernelSize));
    const ctx2 = document.getElementById('outputImageCanvas').getContext('2d');
    const ctxEdge = getCleanCanvas("canvasImgEdge");

    const in1 = new Uint8ClampedArray(valHolder.edgeImage.val_);
    const in2 = new Uint8ClampedArray(valHolder.outputImage1.val_);
    const imageout = new ImageData(in1, canvas.width, canvas.height);
    const edgeImageOut = new ImageData(in2, canvas.width, canvas.height);

    ctx2.putImageData(imageout, 0, 0);
    ctxEdge.ctx.putImageData(edgeImageOut, 0, 0);



    const shapeDemo_ui = document.getElementById('shapeDemo_ui').getContext('2d');
    parseGlobalShapes(shapeDemo_ui, valHolder.shapeStr)
    setTimeout(findMatches(), 0)
}

function main() {
    loadImage('./images/richandmalty.jpg');
    copyimagetocpp();
}

