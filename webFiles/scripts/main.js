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

function copyimagetocpp() {
    // Load image
    // Make canvas same size as image
    const canvas = document.getElementById('shapeDemo');
    const ctx = canvas.getContext('2d');
    const image = ctx.getImageData(0, 0, canvas.width, canvas.height);

    const p = api.create_buffer(canvas.width, canvas.height);
    const p2 = api.create_buffer(canvas.width, canvas.height);
    const p3 = api.create_buffer_zero(8, 1);
    Module.HEAP8.set(image.data, p);
    const p4 = api.encode(p, p2, p3, canvas.width, canvas.height);
    const resultView = new Uint8ClampedArray(Module.HEAP8.buffer, p2, canvas.width*canvas.height*4);
    const resultView4 = new Uint8ClampedArray(Module.HEAP8.buffer, p3, 8);
    console.log("resultView4");
    console.log(resultView4);
    const resultView2 = new Uint8ClampedArray(Module.HEAP8.buffer, p4, resultView4);
    var string = new TextDecoder("utf-8").decode(resultView2);
    console.log(string);

    const canvas2 = document.getElementById('outputImageCanvas');
    const ctx2 = canvas2.getContext('2d');
    const imageout = new ImageData(resultView, canvas.width, canvas.height);
    ctx2.putImageData(imageout, 0, 0);
    const c_shapeDemo = getCleanCanvas("shapeDemo");
    c_shapeDemo.ctx.putImageData(imageout, 0, 0);
    api.destroy_buffer(p);
    api.destroy_buffer(p2);
    api.destroy_buffer(p3);
}

function main() {
    loadImage('./images/richandmalty.jpg')
    copyimagetocpp();
}

