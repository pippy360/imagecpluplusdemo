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
}

let global_shapes = [];

function drawshapefromlist(index) {
    const can = getCleanUICanvas("shapeDemo");
    const ctxEdge = getCleanUICanvas("canvasImgEdge");
    drawPolyFull(can.ctx_ui, global_shapes[index]);
    drawPolyFull(ctxEdge.ctx_ui, global_shapes[index]);
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
        el.innerHTML = `<div class='shapeListEl' onmouseover="drawshapefromlist(${i})" id='shapeListEl${i}'>${opt}</div>`;
        list.appendChild(el);
    }
}

function copyimagetocpp() {
    const c_shapeDemo = getCanvas("shapeDemo");

    // Load image
    // Make canvas same size as image
    const canvas = document.getElementById('shapeDemo');
    const ctx = canvas.getContext('2d');
    const image = ctx.getImageData(0, 0, canvas.width, canvas.height);

    const p = api.create_buffer(canvas.width, canvas.height);
    const p2 = api.create_buffer(canvas.width, canvas.height);
    const imageEdgeBufferPtr = api.create_buffer(canvas.width, canvas.height);
    const p3 = api.create_buffer_zero(8, 1);

    Module.HEAP8.set(image.data, p);
    let kernelSize = parseInt(document.getElementById("cannyKernelSize").value)+1;
    kernelSize += 1-(kernelSize%2);
    console.log("kernelSize")
    console.log(kernelSize)
    const ratioSize = document.getElementById("cannyRatio").value;
    console.log(kernelSize);
    console.log(ratioSize);
    const p4 = api.encode(p, p2, p3, imageEdgeBufferPtr, canvas.width, canvas.height, 100, ratioSize, kernelSize);
    const imageEdgeBuffer = new Uint8ClampedArray(Module.HEAP8.buffer, imageEdgeBufferPtr, canvas.width*canvas.height*4);
    const resultView = new Uint8ClampedArray(Module.HEAP8.buffer, p2, canvas.width*canvas.height*4);
    const resultView4 = byteArrayToLong(new Uint8ClampedArray(Module.HEAP8.buffer, p3, 8));
    console.log("resultView4");
    console.log(resultView4);
    const resultView2 = new Uint8ClampedArray(Module.HEAP8.buffer, p4, resultView4);
    var string = new TextDecoder("utf-8").decode(resultView2);
    // console.log("string: " + string);

    const ctx2 = document.getElementById('outputImageCanvas').getContext('2d');
    const ctxEdge = getCleanCanvas("canvasImgEdge");
    const imageout = new ImageData(resultView, canvas.width, canvas.height);
    const edgeImageOut = new ImageData(imageEdgeBuffer, canvas.width, canvas.height);
    ctx2.putImageData(imageout, 0, 0);
    ctxEdge.ctx.putImageData(edgeImageOut, 0, 0);
    api.destroy_buffer(p);
    api.destroy_buffer(p2);
    api.destroy_buffer(p3);
    api.destroy_buffer(p4);
    api.destroy_buffer(imageEdgeBufferPtr);

    const shapeDemo_ui = document.getElementById('shapeDemo_ui').getContext('2d');
    parseGlobalShapes(shapeDemo_ui, string)
}

function main() {
    loadImage('./images/richandmalty.jpg');
    copyimagetocpp();
}

