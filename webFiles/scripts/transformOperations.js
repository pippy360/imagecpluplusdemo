//include Jquery
//include matrixMath

const INTERACTIVE_CANVAS_OVERLAY_ID = "lookupCanvas_ui";
const DATABASE_CANVAS_OVERLAY_ID = "databaseCanvas_ui";

const enum_TransformationOperation = {
    TRANSLATE: 1,
    UNIFORM_SCALE: 2,
    NON_UNIFORM_SCALE: 3,
    ROTATE: 4,
    CROP: 5,
    SKEW_X: 6,
    SKEW_Y: 7,
};

function convertTransformationObjectToTransformationMatrix(transformations, shapeCenter) {
    // if (!shapeCenter) {
    //     shapeCenter = transformations.transformationCenterPoint;
    // }

    // var transformationCenterPoint = transformations.transformationCenterPoint;
    let ret = getIdentityMatrix();

    ret = matrixMultiply(getTranslateMatrix_point(shapeCenter, -1), ret);

    //Scale
    ret = matrixMultiply(transformations.directionalScaleMatrix, ret);

    //Rotate
    ret = matrixMultiply(getRotationMatrix(transformations.rotation), ret);

    ret = matrixMultiply(getScaleMatrix(transformations.uniformScale, transformations.uniformScale), ret);


    //Rotate
    ret = matrixMultiply(getSkewMatrix(transformations.skew_x, transformations.skew_y), ret);

    ret = matrixMultiply(getTranslateMatrix_point(shapeCenter, 1), ret);

    //Translate
    ret = matrixMultiply(getTranslateMatrix_point(transformations.translate, -1), ret);

    return ret;
}

function newLayer(layerImage, colour) {
    return {
        nonTransformedImageOutline: buildRect(layerImage.width, layerImage.height),
        image: layerImage,
        appliedTransformationsMat: getIdentityMatrix(),
        visible: true,
        layerColour: [0, 0, 0], //used for canvas UI overlay elements
        colour: colour//used for UI elements
    };
}

let interactiveCanvasState = {
    activeLayer: null,
    layers : []
};

let databaseCanvasState = {
    activeLayer: null,
    layers : []
};

function getIdentityTransformations() {
    return {
        transformationCenterPoint: [0, 0],
        uniformScale: 1,
        directionalScaleMatrix: getIdentityMatrix(),
        rotation: 0,
        skew_y: 0,
        skew_x: 0,
        translate: [0, 0]
    };
}

let g_transformState = {
    activeCanvas: interactiveCanvasState,
    interactiveCanvasState: interactiveCanvasState,
    databaseCanvasState: databaseCanvasState,
	currentTranformationOperationState: enum_TransformationOperation.TRANSLATE,
    isMouseDownAndClickedOnCanvas: false,
    temporaryAppliedTransformations: getIdentityTransformations(),
};

function wipeTemporaryAppliedTransformations() {
    g_transformState.temporaryAppliedTransformations = getIdentityTransformations();
}

function setCurrnetOperation(newState) {
    g_transformState.currentTranformationOperationState = newState;
    applyTransformationEffects(newState);
}

function reset() {
    var saved = g_transformState.currentTranformationOperationState;
    g_globalState = buildGlobalState();
    setCurrnetOperation(saved);
    draw();
}

function applyTransformationEffects(state) {
    if (state === enum_TransformationOperation.TRANSLATE) {
        $(".twoCanvasWrapper").addClass("move");
    } else {
        $(".twoCanvasWrapper").removeClass("move");
    }
}

function handleMouseMoveTranslate(pageMouseDownPosition, pageMousePosition, globalState) {
    var translateDelta = minusTwoPoints(pageMouseDownPosition, pageMousePosition);
    globalState.temporaryAppliedTransformations.translate = translateDelta;
}

function getDirectionalScaleMatrix(scaleX, scaleY, direction) {
    var ret = getIdentityMatrix();
    ret = matrixMultiply(ret, getRotationMatrix(direction));
    ret = matrixMultiply(ret, getScaleMatrix(scaleX, scaleY));
    ret = matrixMultiply(ret, getRotationMatrix(-direction));
    return ret;
}

function handleMouseMoveNonUniformScale(pageMouseDownPosition, pageMousePosition, globalState) {
    var mouseDownPoint = pageMouseDownPosition;
    var y = (pageMousePosition[1] - mouseDownPoint[1]);
    var x = (pageMousePosition[0] - mouseDownPoint[0]);

    var extraRotation = Math.atan2(y, x) * (180.0 / Math.PI) * -1;
    if (extraRotation < 0) {
        extraRotation = (360 + (extraRotation));
    }
    direction = extraRotation % 360;
    scale = Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2));
    scale += 50;//skip all the fractions, 1 is the minimum scale
    scale /= 50;
    scaleMatrix = getDirectionalScaleMatrix(Math.sqrt(scale), 1 / Math.sqrt(scale), -direction);
    globalState.temporaryAppliedTransformations.directionalScaleMatrix = scaleMatrix;
}

function handleMouseMoveUniformScale(pageMouseDownPosition, pageMousePosition, globalState) {
    var mouseDownPoint = pageMouseDownPosition;
    var y = (pageMousePosition[1] - mouseDownPoint[1]);
    // var x = (pageMousePosition.x - mouseDownPoint.x);

    scale = y;//(Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2)));

    if (y > 0) {
        scale += 100;
        scale = 1 / (scale / 100);
    } else {
        scale *= -1;//make y positive
        scale += 100;
        scale /= 100;
    }

    globalState.temporaryAppliedTransformations.uniformScale = scale;
}

function handleMouseMoveRotate(pageMouseDownPosition, pageMousePosition, globalState) {
    const y = (pageMousePosition[1] - pageMouseDownPosition[1]);
    const x = (pageMousePosition[0] - pageMouseDownPosition[0]);

    let extraRotation = Math.atan2(y, x) * (180.0 / Math.PI);
    if (extraRotation < 0) {
        extraRotation = (360 + (extraRotation));
    }
    extraRotation = extraRotation % 360;
    globalState.temporaryAppliedTransformations.rotation = extraRotation;
}

function handleMouseMoveSkewX(pageMouseDownPosition, pageMousePosition, globalState) {
    const y = (pageMousePosition[1] - pageMouseDownPosition[1]);
    const x = (pageMousePosition[0] - pageMouseDownPosition[0]);

    let extraRotation = Math.atan2(y, x) * (180.0 / Math.PI);
    if (extraRotation < 0) {
        extraRotation = (360 + (extraRotation));
    }
    extraRotation = extraRotation % 360;
    globalState.temporaryAppliedTransformations.skew_x = extraRotation;
}

function handleMouseMoveSkewY(pageMouseDownPosition, pageMousePosition, globalState) {
    const y = (pageMousePosition[1] - pageMouseDownPosition[1]);
    const x = (pageMousePosition[0] - pageMouseDownPosition[0]);

    let extraRotation = Math.atan2(y, x) * (180.0 / Math.PI);
    if (extraRotation < 0) {
        extraRotation = (360 + (extraRotation));
    }
    extraRotation = extraRotation % 360;
    globalState.temporaryAppliedTransformations.skew_y = extraRotation;
}

function handleMouseMoveCrop(mousePosition, activeLayer) {
    var invMat = math.inv(activeLayer.appliedTransformationsMat);
    var keypointMat = convertSingleKeypointToMatrix(mousePosition);
    var transformedPointMat = applyTransformationMatToSingleKeypoint(keypointMat, invMat);
    var transformedPoint = convertSingleMatrixKeypoinToKeypointObject(transformedPointMat);
    activeLayer.nonTransformedImageOutline.push(transformedPoint);
}

function canvasMouseDownEvent(pageMousePosition, canvasMousePosition) {
    if (g_transformState == null) {
        return;
    }

    g_transformState.isMouseDownAndClickedOnCanvas = true;
    handleMouseDownOnCanvas(pageMousePosition, canvasMousePosition);
}


function handleMouseUp() {
    switch (g_transformState.currentTranformationOperationState) {
        case enum_TransformationOperation.TRANSLATE:
            break;
        case enum_TransformationOperation.NON_UNIFORM_SCALE:
            break;
        case enum_TransformationOperation.UNIFORM_SCALE:
            break;
        case enum_TransformationOperation.ROTATE:
            break;
        case enum_TransformationOperation.SKEW_X:
            break;
        case enum_TransformationOperation.SKEW_Y:
            break;
        case enum_TransformationOperation.CROP:
            return;//FIXME:
            var activeLayer = g_transformState.activeCanvas.activeLayer;
            handleMouseUpCrop(activeLayer);
            break;
        default:
            console.log("ERROR: Invalid state.");
            break;
    }

    wipeTemporaryAppliedTransformations();

}

function drawRotationEffect(pageMousePosition) {
    if (g_transformState.currentTranformationOperationState == enum_TransformationOperation.ROTATE
        || g_transformState.currentTranformationOperationState == enum_TransformationOperation.SKEW_X
        || g_transformState.currentTranformationOperationState == enum_TransformationOperation.SKEW_Y)
    {
        if (g_transformState.isMouseDownAndClickedOnCanvas) {
            _drawRotationUIElement(
                document.getElementById("shapeDemo_ui").getContext("2d"),
                g_transformState.pageMouseDownPosition,
                pageMousePosition,
                g_transformState.temporaryAppliedTransformations.transformationCenterPoint);
        }
    }
}

function handleMouseMoveOnDocument(pageMousePosition) {
    switch (g_transformState.currentTranformationOperationState) {
        case enum_TransformationOperation.TRANSLATE:
            handleMouseMoveTranslate(g_transformState.pageMouseDownPosition, pageMousePosition, g_transformState);
            break;
        case enum_TransformationOperation.NON_UNIFORM_SCALE:
            handleMouseMoveNonUniformScale(g_transformState.pageMouseDownPosition, pageMousePosition, g_transformState);
            break;
        case enum_TransformationOperation.UNIFORM_SCALE:
            handleMouseMoveUniformScale(g_transformState.pageMouseDownPosition, pageMousePosition, g_transformState);
            break;
        case enum_TransformationOperation.ROTATE:
            handleMouseMoveRotate(g_transformState.pageMouseDownPosition, pageMousePosition, g_transformState);
            break;
        case enum_TransformationOperation.SKEW_X:
            handleMouseMoveSkewX(g_transformState.pageMouseDownPosition, pageMousePosition, g_transformState);
            break;
        case enum_TransformationOperation.SKEW_Y:
            handleMouseMoveSkewY(g_transformState.pageMouseDownPosition, pageMousePosition, g_transformState);
            break;
        case enum_TransformationOperation.CROP:
            //ignore, handled in canvas on mouse move function
            break;
        default:
            console.log("ERROR: Invalid state.");
            break;
    }

    //some transformation use the clicked canvas position as the center of the transformation
    const clickedPosition = g_transformState.temporaryAppliedTransformations.transformationCenterPoint;
    const temporaryAppliedTransformationsMat = convertTransformationObjectToTransformationMatrix(g_transformState.temporaryAppliedTransformations, clickedPosition);

    const savedLayerMat = g_transformState.transformationMatBeforeTemporaryTransformations;
    // activeLayer.appliedTransformations = matrixMultiply(temporaryAppliedTransformationsMat, savedLayerMat);
    g_transformState.activeCanvas.activeLayer.appliedTransformationsMat = matrixMultiply(temporaryAppliedTransformationsMat, savedLayerMat);
}

function drawLayerImageOutline(ctx, imageOutlinePolygon) {
    if (imageOutlinePolygon.length === 0) {
        return;
    }

    ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
    ctx.beginPath();

    ctx.moveTo(imageOutlinePolygon[0].x, imageOutlinePolygon[0].y);
    for (var i = 1; i < imageOutlinePolygon.length; i++) {//i = 1 to skip first point
        var currentPoint = imageOutlinePolygon[i];
        ctx.lineTo(currentPoint.x, currentPoint.y);
    }
    ctx.closePath();
    ctx.lineWidth = 2;
    ctx.strokeStyle = '#2196F3';
    ctx.stroke();
}

function handleMouseMoveOnCanvas(canvasMousePosition) {
    switch (g_transformState.currentTranformationOperationState) {
        case enum_TransformationOperation.TRANSLATE:
            //do nothing
            break;
        case enum_TransformationOperation.NON_UNIFORM_SCALE:
            //do nothing
            break;
        case enum_TransformationOperation.UNIFORM_SCALE:
            //do nothing
            break;
        case enum_TransformationOperation.ROTATE:
            //do nothing
            break;
        case enum_TransformationOperation.CROP:
            return;//FIXME:
            var activeLayer = g_transformState.activeCanvas.activeLayer;
            handleMouseMoveCrop(canvasMousePosition, g_transformState);
            break;
        default:
            console.log("ERROR: Invalid state.");
            break;
    }
}

function handleMouseDownCrop(activeLayer) {
    //The nonTransformedImageOutline is never allowed to be an empty list
    //so onMouseUp if the nonTransformedImageOutline is still empty then
    //it is replaced with the outline of the image with no cropping
    activeLayer.nonTransformedImageOutline = [];
}

function getActiveLayerWithCanvasPosition(canvasMousePosition, layers, noMatchReturnValue) {

    for (let i = 0; i < layers.length; i++) {
        let layer = layers[i];
        // Apply our transformation matrix to the non transformed image outline
        let imageOutline = applyTransformationMatrixToAllPoints(
            layer.nonTransformedImageOutline, layer.appliedTransformationsMat);
        // Take the cropping shape
        if (isPointInPolygon(canvasMousePosition, imageOutline)) {
            return layer;
        }
    }
    return noMatchReturnValue;

}

function handleMouseDownOnCanvas(pageMousePosition, canvasMousePosition) {

    g_transformState.pageMouseDownPosition = pageMousePosition;
    g_transformState.temporaryAppliedTransformations.transformationCenterPoint = canvasMousePosition;

    let currentActiveLayer = g_transformState.activeCanvas.activeLayer;
    const clickedActiveLayer = getActiveLayerWithCanvasPosition(canvasMousePosition,
        g_transformState.activeCanvas.layers, currentActiveLayer);

    g_transformState.activeCanvas.activeLayer = clickedActiveLayer;
    g_transformState.transformationMatBeforeTemporaryTransformations = clickedActiveLayer.appliedTransformationsMat;

    switch (g_transformState.currentTranformationOperationState) {
        case enum_TransformationOperation.TRANSLATE:
            //do nothing
            break;
        case enum_TransformationOperation.NON_UNIFORM_SCALE:
            //do nothing
            break;
        case enum_TransformationOperation.UNIFORM_SCALE:
            //do nothing
            break;
        case enum_TransformationOperation.ROTATE:
            //do nothing
            break;
        case enum_TransformationOperation.CROP:
            //FIXME: handleMouseDownCrop(clickedActiveLayer);
            break;
        default:
            console.log("ERROR: Invalid state.");
            break;
    }
}

function mouseMoveOnDocumentEvent(pageMousePosition) {
    if (g_transformState != null && g_transformState.isMouseDownAndClickedOnCanvas) {
        g_transformState.referenceImageHighlightedTriangle = null;
        // copyimagetocpp();

        // g_transformState.activeCanvas.imageOutlineHighlightLayer = g_transformState.activeCanvas.activeLayer;
        handleMouseMoveOnDocument(pageMousePosition);
        //draw();
        //clearOutputListAndWipeCanvas();//FIXME:
        draw(pageMousePosition);
    }
}

function mouseUpEvent() {
    if (g_transformState != null && g_transformState.isMouseDownAndClickedOnCanvas) {
        //FIXME: we actually only need to update one heap
        updateDatabaseCanvasHeap();
        updateLookupCanvasHeap();

        draw();
        handleMouseUp();
        g_transformState.isMouseDownAndClickedOnCanvas = false;
        loadEdgeImages();
        findMatches();
    }
}


function canvasMouseDownEvent(pageMousePosition, canvasMousePosition) {
    if (g_transformState == null) {
        return;
    }

    g_transformState.isMouseDownAndClickedOnCanvas = true;
    handleMouseDownOnCanvas(pageMousePosition, canvasMousePosition);
}

function canvasMouseMoveEvent(canvasMousePosition) {
    if (g_transformState == null) {//hack
        return;
    }
    // const layers = g_transformState.interactiveCanvasState.layers;
    // const canvasContext = g_transformState.interactiveCanvasState.imageOutlineLayerCanvasContext;
    //
    // g_transformState.interactiveCanvasState.imageOutlineHighlightLayer = getActiveLayerWithCanvasPosition(canvasMousePosition, layers, null);

    // if (g_transformState == null || g_transformState.activeCanvas != g_transformState.interactiveCanvasState) {
    //     return;
    // }

    if (g_transformState.isMouseDownAndClickedOnCanvas) {
        handleMouseMoveOnCanvas(canvasMousePosition);
        clearLowerUi();
    }
}


function _drawRotationUIElement(ctx, pageMouseDownPosition, pageMousePosition, outPos) {
    const dx = pageMousePosition[0] - pageMouseDownPosition[0];
    const dy = pageMousePosition[1] - pageMouseDownPosition[1];

    const targetDist = 100;Math.sqrt(dx**2 + dy**2);
    const resAngle = ((Math.atan2(dy, dx)));
    const resPoint = [ outPos[0] + targetDist*Math.cos(resAngle), outPos[1] + targetDist*Math.sin(resAngle) ];
    const side = [ outPos[0] + 100, outPos[1] ];

    drawline_m(ctx, [outPos, resPoint], 'red');
    drawline_m(ctx, [outPos, side], 'red');

    ctx.beginPath();

    if (resAngle > 0)
        ctx.arc(outPos[0], outPos[1], 50, 0, resAngle);
    else
        ctx.arc(outPos[0], outPos[1], 50, resAngle, 0);

    ctx.stroke();
}

//layers

function buildRect(x2, y2) {
    return [
        [0, 0],
        [x2, 0],
        [x2, y2],
        [0, y2]
    ]

}

let g_initImages = false;
const g_src = './images/basicshapes.png'
var g_img = new Image();
g_img.src = g_src;

function addLayer(canvasState, img) {
    canvasState.layers.push(newLayer(img))
    if (canvasState.layers.length == 1) {
        canvasState.activeLayer = canvasState.layers[0];
    }
}

async function initImages() {
    // We're guaranteed images are already loaded
    addLayer(g_transformState.interactiveCanvasState, g_img);
    addLayer(g_transformState.databaseCanvasState, g_img);
}

var g_globalState = {
    canvasClickLocation: {x: .5, y: .5},
    inputImage1Mat: null,
    inputImage2Mat: null,
};

function updateClickAndSeeImage() {

    const lookupCanvas = getCanvas("lookupCanvas");
    const c_clickandseeImageLeft = getCleanCanvas("clickandseeImageLeft");
    c_clickandseeImageLeft.ctx.drawImage(lookupCanvas.c, 0, 0);

    const databaseCanvas = getCanvas("databaseCanvas");
    const c_clickandseeImageRight = getCleanCanvas("clickandseeImageRight");
    c_clickandseeImageRight.ctx.drawImage(databaseCanvas.c, 0, 0);

}

function draw() {
    console.log("draw called");

    if (!g_initImages) {
        initImages();
        g_initImages = true;
    }

    // window.history.pushState("object or string", "Title", "index.html?point=" + g_globalState.canvasClickLocation.x + ","
    //     + g_globalState.canvasClickLocation.y + "&image=" + g_src + "&appliedTransformationsMat="
    //     + JSON.stringify(g_transformState.appliedTransformationsMat)+"" );

    const c_lookupCanvas = getCleanCanvas("lookupCanvas");
    const c_databaseCanvas = getCleanCanvas("databaseCanvas");

    for (let i = 0; i < g_transformState.interactiveCanvasState.layers.length; i++){
        const layer = g_transformState.interactiveCanvasState.layers[i];
        const transMat = layer.appliedTransformationsMat;
        drawImageWithTransformations(c_lookupCanvas.ctx, layer.image, transMat);
    }
    for (let i = 0; i < g_transformState.databaseCanvasState.layers.length; i++){
        const layer = g_transformState.databaseCanvasState.layers[i];
        const transMat = layer.appliedTransformationsMat;
        drawImageWithTransformations(c_databaseCanvas.ctx, layer.image, transMat);
    }

    let width = c_lookupCanvas.c.width;
    let height = c_lookupCanvas.c.height;
    // drawline_m(c_lookupCanvas.ctx_ui, [[0, height/2], [width, height/2]], 'red');
    // drawline_m(c_lookupCanvas.ctx_ui, [[width/2, 0], [width/2, height]], 'red');

    //drawRotationEffect(pageMousePosition);

    updateClickAndSeeImage();
}

//hooks
$(document).mousedown(function (e) {
    //ignore
});

$(document).mousemove(function (e) {
    var pageMousePosition = getCurrentPageMousePosition(e);
    mouseMoveOnDocumentEvent(pageMousePosition);
});

$(document).bind( "touchmove", function (e) {
    const pageMousePosition = [
        e.originalEvent.touches[0].pageX,
        e.originalEvent.touches[0].pageY
    ];
    if (g_transformState != null && g_transformState.isMouseDownAndClickedOnCanvas) {
        e.preventDefault();
    }
    mouseMoveOnDocumentEvent(pageMousePosition);
});

$(document).mouseup(function (e) {
    mouseUpEvent();
});

$(document).bind( "touchend", function (e) {
    mouseUpEvent()
});

$("#" + INTERACTIVE_CANVAS_OVERLAY_ID).mousedown(function (e) {
    e.preventDefault();

    g_transformState.activeCanvas = g_transformState.interactiveCanvasState;

    var canvasElem = $("#" + INTERACTIVE_CANVAS_OVERLAY_ID)[0];
    const pageMousePosition = getCurrentPageMousePosition(e);
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    canvasMouseDownEvent(pageMousePosition, canvasMousePosition);
});

$(document).on('touchstart', "#" + INTERACTIVE_CANVAS_OVERLAY_ID, function(e) {
    e.preventDefault();
    const pageMousePosition = [
        e.originalEvent.touches[0].pageX,
        e.originalEvent.touches[0].pageY
    ];
    var canvasElem = $("#" + INTERACTIVE_CANVAS_OVERLAY_ID)[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    canvasMouseDownEvent(pageMousePosition, canvasMousePosition);
});


$("#" + INTERACTIVE_CANVAS_OVERLAY_ID).mousemove(function (e) {
    var canvasElem = $("#" + INTERACTIVE_CANVAS_OVERLAY_ID)[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);

    canvasMouseMoveEvent(canvasMousePosition);
});

$(document).on('touchmove', "#" + INTERACTIVE_CANVAS_OVERLAY_ID, function(e) {
    e.preventDefault();
    var canvasElem = $("#" + INTERACTIVE_CANVAS_OVERLAY_ID)[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    canvasMouseMoveEvent(canvasMousePosition);
});

$("#" + INTERACTIVE_CANVAS_OVERLAY_ID).mouseup(function (e) {
    if (g_transformState == null) {
        return;
    }
    //ignore
});



$("#" + DATABASE_CANVAS_OVERLAY_ID).mousedown(function (e) {
    e.preventDefault();

    g_flushCache = true;

    g_transformState.activeCanvas = g_transformState.databaseCanvasState;

    var canvasElem = $("#" + DATABASE_CANVAS_OVERLAY_ID)[0];
    const pageMousePosition = getCurrentPageMousePosition(e);
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    canvasMouseDownEvent(pageMousePosition, canvasMousePosition);
});

$(document).on('touchstart', "#" + DATABASE_CANVAS_OVERLAY_ID, function(e) {
    e.preventDefault();
    const pageMousePosition = [
        e.originalEvent.touches[0].pageX,
        e.originalEvent.touches[0].pageY
    ];
    var canvasElem = $("#" + DATABASE_CANVAS_OVERLAY_ID)[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    canvasMouseDownEvent(pageMousePosition, canvasMousePosition);
});


$("#" + DATABASE_CANVAS_OVERLAY_ID).mousemove(function (e) {
    var canvasElem = $("#" + DATABASE_CANVAS_OVERLAY_ID)[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);

    canvasMouseMoveEvent(canvasMousePosition);
});

$(document).on('touchmove', "#" + DATABASE_CANVAS_OVERLAY_ID, function(e) {
    e.preventDefault();
    var canvasElem = $("#" + DATABASE_CANVAS_OVERLAY_ID)[0];
    const canvasMousePosition = getCurrentCanvasMousePosition(e, canvasElem);
    canvasMouseMoveEvent(canvasMousePosition);
});

$("#" + DATABASE_CANVAS_OVERLAY_ID).mouseup(function (e) {
    if (g_transformState == null) {
        return;
    }
    //ignore
});
