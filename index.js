    var scrcont = document.getElementById("screen-container");
    document.getElementById("screen").height = scrcont.clientHeight;
document.getElementById("screen").width = scrcont.clientWidth;
document.getElementById('scr-size').innerHTML = scrcont.clientWidth + ' x ' +
    scrcont.clientHeight;


    function setpixelated(context){
    context['imageSmoothingEnabled'] = false;       /* standard */
    context['mozImageSmoothingEnabled'] = false;    /* Firefox */
    context['oImageSmoothingEnabled'] = false;      /* Opera */
    context['webkitImageSmoothingEnabled'] = false; /* Safari */
    context['msImageSmoothingEnabled'] = false;     /* IE */
}

const ctx = document.getElementById('screen').getContext('2d');
setpixelated(ctx);
var width = scrcont.clientWidth;
var height = scrcont.clientHeight;
let image = ctx.createImageData(width, height);

let stat = document.getElementById('stat').getContext('2d');

var module,
exports;
var nextFrame, waScene, waFuns;

webassembly
    .load("main.wasm", {'initialMemory': 1, 'imports': {
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
    waFuns = exports;

    let byteSize = width * height * 4;
    var pointer = exports.heap_alloc( byteSize );
    var usub = new Uint8ClampedArray(module.memory.buffer, pointer, byteSize);
    var img = new ImageData(usub, width, height);

    var scene = exports.scene_init(width, height, pointer, null);
    waScene = scene;

    //exports.bufdraw(pointer, width, height);
    //ctx.putImageData(img, 100, 100);

    var i = 0;
    var lastCalledTime = Date.now();
    var lastFtpUpdate = Date.now() - 1000;
    var maxFps = 60;
    var delay = 0;
    var lastStatFps = maxFps;
    function frame(render_mode) {
        render_mode = (render_mode === undefined || render_mode === null) ?
            1 * document.getElementById('face-mode').checked +
            2 * document.getElementById('wireframe-mode').checked +
            4 * document.getElementById('zbuf-mode').checked +
            8 * document.getElementById('grid-mode').checked +
            16 * document.getElementById('xyz-mode').checked +
            32 * document.getElementById('norms-mode').checked +
            64 * document.getElementById('verts-mode').checked +
            128 * document.getElementById('lights-mode').checked +
            256 * document.getElementById('colored-mode').checked +
            1 - 1 : render_mode;
        //ctx.clearRect(0, 0, scrcont.clientHeight, scrcont.clientWidth);
        //setInterval(1000);
        /*for (var j = 0; j < scr бесcont.clientHeight; j++) {
            styles = ["red", "green", "blue", "black", "grey", "white"];
            ctx.fillStyle = styles[(i+j) % styles.length];
            exports.fillit(10, j, 1000, j + 1);
        }
        i = ++i % styles.length;*/
        try {
            var start = new Date().getTime();

            try {
            exports.frame(scene, render_mode);
            } catch (e) {
                alert(e);
            }

            if (!usub.byteLength)
            {
                usub = new Uint8ClampedArray(module.memory.buffer, pointer, byteSize);
                img = new ImageData(usub, width, height);
            }
            ctx.putImageData(img, 0, 0);
            //exports.draw(scrcont.clientHeight);
            var render = new Date().getTime() - start;
            exports.clear(scene);
            var clear = new Date().getTime() - start - render;
            //console.log(end - start);
        } catch (e) {
            alert(e);
        }

        let now = Date.now();
        let delta = (now - lastCalledTime)/1000;
        delay = 1000/maxFps - (now - lastCalledTime);
        lastCalledTime = now;
        let fps = 1/delta;

        //stat.fillRect(0, 150-100*fps/maxFps, 2, 2);
        stat.putImageData(stat.getImageData(0, 0, 300-1, 150), 1, 0);
        stat.fillStyle = 'red';
        stat.clearRect(0, 0, 1, 150);
        stat.fillRect(0, 50, 1, 1);
        stat.fillStyle = 'black';
        stat.beginPath();
        stat.moveTo(1, 150-100*lastStatFps/maxFps);
        stat.lineTo(0, 150-100*((lastFtpUpdate + 300 < now) ? fps : lastStatFps)/maxFps);
        stat.stroke();
        if (lastFtpUpdate + 300 < now)
            lastStatFps = fps;

        if (lastFtpUpdate + 500 < now)
        {
            document.getElementById('fps').innerHTML = 'fps: ' + Math.round(fps) +
                '<br />render: '+render+'ms<br /> clear: '+clear+'ms<br />'+
                'frame req delay: ' + (delay > 0 ? Math.round(delay) : 0)+
                'ms<br />max fps: '+maxFps+'<br />Mem usage: '+
                Math.round(module.memory.buffer.byteLength/1024/1024)+'MB '+
                module.memory.buffer.byteLength/1024%1024+'KB<br />';
            lastFtpUpdate = Date.now();
            document.getElementById('logs').innerHTML = '';

        }

        return;
        //requestAnimationFrame(frame);
        if (delay > 0)
        setTimeout(function () {requestAnimationFrame(frame);}, delay);
        else
        requestAnimationFrame(frame);
    };
        nextFrame = frame;
        frame();


        var events = {};
        addEventListener("keydown", function(e) {
            events[e.keyCode] = true;
            let hor = (events[37] === true ? 1 : 0) + (events[39] === true ? -1 : 0);
            let vert = (events[38] === true ? 1 : 0) + (events[40] === true ? -1 : 0);
            switch(e.keyCode) {
                case 37:  // если нажата клавиша влево
                    break;
                case 38:   // если нажата клавиша вверх
                    break;
                case 39:   // если нажата клавиша вправо
                    break;
                case 40:   // если нажата клавиша вниз
                    break;
            }

            if (hor === 0 && vert === 0)
                return;

            waFuns.move_viewport(waScene, hor/180*3.1415,  vert/180*3.1415, 0, 0);
            if (document.getElementById('fast-mode').checked)
                nextFrame(2 + 128);
            else
                nextFrame();
        });

        addEventListener("keyup", function(e) {
            if (events[e.keyCode] !== true)
                return;

            let allUp = true;
            events[e.keyCode] = false;

            for (let ev in events)
                if (ev === true)
                {
                    allUp = false;
                    break;
                }

            if (allUp)
                nextFrame();
        });
    })
    .catch(e => {alert(e)});
