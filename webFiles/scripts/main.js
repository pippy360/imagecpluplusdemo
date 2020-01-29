async function loadImage(src) {
    // Load image
    const imgBlob = await fetch(src).then(resp => resp.blob());
    const img = await createImageBitmap(imgBlob);
    // Make canvas same size as image
    const canvas = document.createElement('canvas');
    canvas.width = img.width;
    canvas.height = img.height;
    // Draw image onto canvas
    const ctx = canvas.getContext('2d');
    ctx.drawImage(img, 0, 0);
    const image = ctx.getImageData(0, 0, img.width, img.height);
    console.log(image.data);

    const p = api.create_buffer(image.width, image.height);
    const p2 = api.create_buffer(image.width, image.height);
    const p3 = api.create_buffer(8, 1);
    Module.HEAP8.set(image.data, p);
    const p4 = api.encode(p, p2, p3, img.width, img.height);
    const resultView = new Uint8ClampedArray(Module.HEAP8.buffer, p2, image.width*image.height*4);
    const resultView4 = byteArrayToLong(new Uint8ClampedArray(Module.HEAP8.buffer, p3, 8));
    console.log("resultView4");
    console.log(resultView4);
    const resultView2 = new Uint8ClampedArray(Module.HEAP8.buffer, p4, resultView4);
    var string = new TextDecoder("utf-8").decode(resultView2);
    console.log(string);

    const canvas2 = document.getElementById('outputImageCanvas');
    const ctx2 = canvas2.getContext('2d');
    const imageout = new ImageData(resultView, image.width, image.height);
    ctx2.putImageData(imageout, 0, 0);
}

function main() {
    loadImage('./images/richandmalty.jpg')
}