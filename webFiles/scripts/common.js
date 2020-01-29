// #     #                         ###
// #     #  ####  ###### #####      #  #    # #####  #    # #####
// #     # #      #      #    #     #  ##   # #    # #    #   #
// #     #  ####  #####  #    #     #  # #  # #    # #    #   #
// #     #      # #      #####      #  #  # # #####  #    #   #
// #     # #    # #      #   #      #  #   ## #      #    #   #
//  #####   ####  ###### #    #    ### #    # #       ####    #
//user input


function _getCanvas(id, cleanBase, cleanUI) {
    let c = document.getElementById(id);
    let ctx = c.getContext("2d");
    if (cleanBase)
        ctx.clearRect(0, 0, c.width, c.height);
    let c_ui = document.getElementById(id + "_ui");
    let ctx_ui = c_ui.getContext("2d");
    if (cleanUI)
        ctx_ui.clearRect(0, 0, c.width, c.height);
    return {
        c: c,
        ctx: ctx,
        c_ui: c_ui,
        ctx_ui: ctx_ui,
    }
}

function getCanvas(id) {
    return _getCanvas(id, false, false)
}

function getCleanUICanvas(id) {
    return _getCanvas(id, false, true)
}

function getCleanCanvas(id) {
    return _getCanvas(id, true, true)
}

function getCurrentPageMousePosition(e) {
    return [
        e.pageX,
        e.pageY
    ];
}

function getCurrentCanvasMousePosition(e, canvasElem) {
    if (e.originalEvent.changedTouches != null && canvasElem != null) {
        var rect = canvasElem.getBoundingClientRect();
        return [
            e.originalEvent.changedTouches[0].clientX - rect.left,
            e.originalEvent.changedTouches[0].clientY - rect.top
        ];
    } else if (e.clientX || e.clientX === 0 && canvasElem != null) {
        var rect = canvasElem.getBoundingClientRect();
        return [
            e.clientX - rect.left,
            e.clientY - rect.top
        ];
    } else {
        console.log("Error: Invalid state");
    }
}

function canvasTransform(ctx, mat) {
    ctx.transform(mat[0][0], mat[1][0], mat[0][1], mat[1][1], mat[0][2], mat[1][2]);
}
