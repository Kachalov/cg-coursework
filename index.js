    var scrcont = document.getElementById("screen-container");
    document.getElementById("screen").height = scrcont.clientHeight;
document.getElementById("screen").width = scrcont.clientWidth;


    function setpixelated(context){
    context['imageSmoothingEnabled'] = false;       /* standard */
    context['mozImageSmoothingEnabled'] = false;    /* Firefox */
    context['oImageSmoothingEnabled'] = false;      /* Opera */
    context['webkitImageSmoothingEnabled'] = false; /* Safari */
    context['msImageSmoothingEnabled'] = false;     /* IE */
}

const ctx = document.querySelector('canvas').getContext('2d');
setpixelated(ctx);
var width = scrcont.clientWidth;
var height = scrcont.clientHeight;
let image = ctx.createImageData(width, height);

var module,
exports;

webassembly
    .load("main.wasm", {'imports': {
        clearRect: ctx.clearRect.bind(ctx),
        fillRect: ctx.fillRect.bind(ctx),
        lineTo: ctx.lineTo.bind(ctx),
        stroke: ctx.stroke.bind(ctx),
        beginPath: ctx.beginPath.bind(ctx),
        pixel_color: color => {ctx.fillStyle = module.memory.getString(color)},
        logs: str => {document.getElementById('logs').innerHTML += module.memory.getString(str) + '</br>'}
        //set_pixel: image.putImageData.bind(ctx),
        //get_pixel: image.getImageData.bind(ctx),
        }})
    .then(function(module_) {
    exports = (module = module_).exports;


    let byteSize = width * height * 4;
    var pointer = exports.heap_alloc( byteSize );
    var usub = new Uint8ClampedArray(module.memory.buffer, pointer, byteSize);
    var img = new ImageData(usub, width, height);

    exports.bufdraw(pointer, width, height);
    ctx.putImageData(img, 100, 100);

    var i = 0;
    var lastCalledTime = Date.now();
    var lastFtpUpdate = Date.now() - 600;
    var maxFps = 30;
    var delay = 0;
    (function frame() {
        //ctx.clearRect(0, 0, scrcont.clientHeight, scrcont.clientWidth);
        //setInterval(1000);
        /*for (var j = 0; j < scrcont.clientHeight; j++) {
            styles = ["red", "green", "blue", "black", "grey", "white"];
            ctx.fillStyle = styles[(i+j) % styles.length];
            exports.fillit(10, j, 1000, j + 1);
        }
        i = ++i % styles.length;*/
        try {
            var start = new Date().getTime();
            exports.bufdraw(pointer, width, height);
            ctx.putImageData(img, 0, 0);
            //exports.draw(scrcont.clientHeight);
            var render = new Date().getTime() - start;
            //console.log(end - start);
        } catch (e) {
            alert(e);
        }

        let delta = (Date.now() - lastCalledTime)/1000;
        delay = 1000/maxFps - (Date.now() - lastCalledTime);
        lastCalledTime = Date.now();
        let fps = 1/delta;


        if (lastFtpUpdate + 500 < Date.now())
        {
            document.getElementById('fps').innerHTML = 'fps: ' + Math.round(fps) + ' render: '+
            render+'ms<br />' + 'delay: ' + Math.round(delay) + 'ms<br />max fps: ' +
            maxFps;
            lastFtpUpdate = Date.now();
            document.getElementById('logs').innerHTML = '';
        }

        //return;
        //requestAnimationFrame(frame);
        if (delay > 0)
        setTimeout(function () {requestAnimationFrame(frame);}, delay);
        else
        requestAnimationFrame(frame);
    })();
    /*var ptr = module.mem.U8[24];
    alert(exports.point(ptr, 10));
    alert(exports.point_init(ptr, 1, 2, 3));
    alert(exports.point_x(ptr));*/
    })
    .catch(e => {alert(e)});
