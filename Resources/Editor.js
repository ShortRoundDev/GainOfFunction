const canv = document.querySelector("canvas");
const ctx = canv.getContext('2d');
const tWidth = 64;
const tHeight = 64;

let xMax = 10;
let yMax = 10;

let grid = [];
let entityGrid = [];
let entities = [];
let totalEnts = 0;
let editor = null;
let player = {
    x: 0,
    y: 0,
    lookDir: 1 // 0 = up, 1 = right, 2 = down, 3 = left
};
//let tileDefs = {};

// Key Codes
const K_UP = 38;
const K_LEFT = 37;
const K_RIGHT = 39;
const K_DOWN = 40;
const K_SHIFT = 16;
const K_CTRL = 17;
const K_P = 80;

let camera = {
    x: 0,
    y: 0
};

let mousePos = {
    x: 0,
    y: 0
};

let keyMap = [];

function main() {
    setInterval(() => {
        ctx.clearRect(0, 0, canv.width, canv.height);
        draw();
        update();
    }, 16);
}

function init() {
    for(let i = 0; i < 256; i++){
        grid[i] = new Array(256);
    }
    for(let i = 0; i < 256; i++){
        entityGrid[i] = new Array(256);
    }
    editor = new EditorState;
    canv.width = window.innerWidth - 512;
    canv.height = window.innerHeight;
    document.addEventListener("keydown", keyDown);
    document.addEventListener("keyup", keyUp);
    canv.addEventListener("click", click);
    canv.addEventListener("contextmenu", editTile);
    canv.addEventListener("mousedown", middleClick);
    canv.addEventListener("mousemove", mouseMove);
    window.addEventListener("resize", resizeCanv);
    ctx.imageSmoothingEnabled = false;

}

function resizeCanv(event) {
    canv.width = window.innerWidth - 512;
    canv.height = window.innerHeight;
    ctx.imageSmoothingEnabled = false;

}

function draw() {
    drawGrid();
    drawTiles();
    drawPlayer();
    drawEntities();
    drawCursor();
}

function update() {
    checkMove();
}

// Events

function keyDown(e) {
    keyMap[e.keyCode] = true;
    if(keyMap[K_CTRL]){
        if(e.keyCode == K_LEFT){
            if(keyMap[K_SHIFT]){
                console.log("left");
                xMax--;
                xMax = Math.max(xMax, 0);
            } else {
                player.lookDir = 3;
            }
        } else if(e.keyCode == K_RIGHT){
            if(keyMap[K_SHIFT]){
                xMax++;
                xMax = Math.min(xMax, 255);
            } else {
            player.lookDir = 1;
            }
        } else if(e.keyCode == K_DOWN){
            if(keyMap[K_SHIFT]){
                yMax++;
                yMax = Math.min(yMax, 255);
            }else{
                player.lookDir = 0;
            }
        } else if(e.keyCode == K_UP){
            if(keyMap[K_SHIFT]){
                yMax--;
                yMax = Math.max(0, yMax);
            }else {
                player.lookDir = 2;
            }
        }
    }
    if(e.keyCode == K_P){
        let x = mousePos.x + camera.x;
        let y = mousePos.y + camera.y;
        let tileX = parseInt(x/tWidth);
        let tileY = parseInt(y/tHeight);
        player.x = tileX;
        player.y = tileY;
    }
}

function keyUp(e) {
    keyMap[e.keyCode] = false;
}

function mouseMove(e) {
    mousePos.x = e.clientX;
    mousePos.y = e.clientY;
}

function click(e) {
    let x = mousePos.x + camera.x;
    let y = mousePos.y + camera.y;
    let tileX = parseInt(x/tWidth);
    let tileY = parseInt(y/tHeight);
    let t = grid[tileX][tileY] ?? new Tile(tileX, tileY);
    idx = "wallTexture";
    switch(editor.layer) {
        case 0: idx = "floorTexture"; break;
        case 1: idx = "wallTexture"; break;
        case 2: idx = "ceilingTexture"; break;
    }
    if(editor.tile !== null && editor.entity === null){
        t[idx] = editor.tile;
        grid[tileX][tileY] = t;
        editor.selectedTile = t;
        console.log(t.wallTexture);
        console.log(editor.layer)
        if(editor.layer === 1 && (t.wallTexture >= 100) && (t.wallTexture <= 104)) {
            t.isDoor = true;
            var key = t.wallTexture - 100;
            if(key === 4)
                t.key = 0;
            else
                t.key = (key + 1);
        } 

        openEditor(t);
    } else if(editor.layer === 1) { // entities can only go on layer 1
        let e = entityGrid[tileX][tileY];
        if(!e){
            totalEnts++;
        }
        console.log(editor.entity + ": " + tileX + ", " + tileY);
        entityGrid[tileX][tileY] = editor.entity;
    }
}

function middleClick(e) {
    let x = mousePos.x + camera.x;
    let y = mousePos.y + camera.y;
    let tileX = parseInt(x/tWidth);
    let tileY = parseInt(y/tHeight);
    
    if(e.button === 1){
        e.preventDefault();
        if(keyMap[K_CTRL]){
            let e = entityGrid[tileX][tileY];
            if(e){
                totalEnts--;
            }
            entityGrid[tileX][tileY] = undefined;
        }else if(keyMap[K_SHIFT]){
            grid[tileX][tileY] = undefined;
        }else{
            switch(editor.layer) {
                case 0: idx = "floorTexture"; break;
                case 1: idx = "wallTexture"; break;
                case 2: idx = "ceilingTexture"; break;
            }
            grid[tileX][tileY][idx] = undefined;
        }
    }
}

function editTile(e) {
    e.preventDefault();
    let x = mousePos.x + camera.x;
    let y = mousePos.y + camera.y;
    let tileX = parseInt(x/tWidth);
    let tileY = parseInt(y/tHeight);

    let tile = grid[tileX][tileY]
    if(!tile) {
        return;
    }
    editor.selectedTile = tile;
    openEditor(tile)
}

// Drawing

function drawGrid() {
    for(let i = 0; i < canv.width; i += tWidth){
        let iMod = i - (camera.x % tWidth);
        ctx.strokeStyle = "black";
        ctx.beginPath()
        ctx.moveTo(iMod, 0);
        ctx.lineTo(iMod, canv.height);
        ctx.stroke();
        for(let j = 0; j < canv.height + tHeight; j += tHeight){
            let jMod = j - (camera.y % tHeight);
            ctx.beginPath();
            ctx.moveTo(0, jMod);
            ctx.lineTo(canv.width, jMod);
            ctx.stroke();
        }
    }

    ctx.beginPath();
    let x = ((xMax * tWidth) - camera.x);
    let y = ((yMax * tHeight) - camera.y);

    ctx.lineWidth = 4;
    ctx.strokeStyle = "cornflowerblue";
    ctx.moveTo(x, 0);
    ctx.lineTo(x, y);
    ctx.moveTo(0, y);
    ctx.lineTo(x, y);
    ctx.stroke();
    ctx.lineWidth = 1;
    ctx.strokeStyle = "black";
}

function drawTiles() {
    for(let i = 0; i < 256; i++){
        for(let j = 0; j < 256; j++){
            let wall = grid[i][j];
            if(!wall)
                continue;
            let idx = 0;
            switch(editor.layer) {
                case 0: idx = wall.floorTexture; break;
                case 1: idx = wall.wallTexture; break;
                case 2: idx = wall.ceilingTexture; break;
            }

            let x = (i * tWidth) - camera.x;
            let y = (j * tHeight) - camera.y;

            ctx.beginPath();
            ctx.lineWidth = 4;
            ctx.strokeStyle = "black";
            ctx.rect(x, y, tWidth, tHeight);
            ctx.stroke();
            ctx.lineWidth = 1;
            const img = editor.getTile(idx);
            if(!img){
                continue;
            }

            ctx.drawImage(img, x, y, tWidth, tHeight);
            if(wall === editor.selectedTile){
                ctx.beginPath();
                ctx.lineWidth = 2;
                ctx.strokeStyle = "rgb(0, 255, 0)";
                ctx.rect(x, y, tWidth, tHeight);
                ctx.stroke();
                ctx.lineWidth = 1;
            }
        }
    }
}

function drawEntities() {
    if(editor.layer !== 1)
        return;
    for(let i = 0; i < 256; i++){
        for(let j = 0; j < 256; j++){
            let entity = entityGrid[i][j];
            if(!entity)
                continue;
            
            let x = (i * tWidth) - camera.x;
            let y = (j * tHeight) - camera.y;
            
            const img = editor.getTile(entity);
            if(!img){
                continue;
            }
            ctx.drawImage(img, x, y, tWidth, tHeight);
        }
    }
}

function drawPlayer() {
    ctx.beginPath();
    let x = (player.x * tWidth) - camera.x + (tWidth/2);
    let y = (player.y * tHeight) - camera.y + (tHeight/2)
    ctx.arc(
        x,
        y,
        tWidth/2 - 4,
        0,
        Math.PI * 2
    );
    ctx.fillStyle = "white";
    ctx.fill();
    ctx.moveTo(x, y);
    switch(player.lookDir) {
        case 0: ctx.lineTo(x, y - tWidth/2 + 4); break;
        case 1: ctx.lineTo(x + tWidth/2 - 4, y); break;
        case 2: ctx.lineTo(x, y + tWidth/2 - 4); break;
        case 3: ctx.lineTo(x - tWidth/2 + 4, y); break;
    }
    ctx.stroke();
}

function drawCursor() {
    let x = mousePos.x + (camera.x % tWidth);
    let y = mousePos.y + (camera.y % tHeight);
    let highlightX = (parseInt(x/tWidth) * tWidth) - (camera.x % tWidth);
    let highlightY = parseInt(y/tHeight) * tHeight - (camera.y % tHeight);;
    ctx.strokeStyle = "red";
    ctx.lineWidth = 3;  
    ctx.lineStyle = "dotted";
    ctx.beginPath();
    ctx.rect(highlightX, highlightY, tWidth, tHeight)
    ctx.stroke();
    ctx.strokeStyle = "black";
    ctx.lineWidth = 1;

}

// Editing

class EditorState {
    constructor(){
        this.layer = 1;
        this.tile = 0;
        this.ceiling = 0;
        this.floor = 0;
        this.entity = null;
        this._tiles = [];
        this.selectedTile = null;
    }

    get isTiling(){ return this.entity == null && this.tile != null; }
    get isEntityPlacing() { return !isTiling(); }

    addTile(x, t) {
        console.log(x);
        this._tiles[x] = t;
        console.log(this._tiles[1000]);
    }

    getTile(x){
        return this._tiles[x];
    }

}

class Tile {
    constructor(x, y) {
        this.x = x;
        this.y = y;
        this.wallTexture = 0;
        this.floorTexture = 0;
        this.ceilingTexture = 0;
        this.zone = 0;
        this.isDoor = false;
        this.key = 0;
    }
}

function checkMove() {
    let speed = keyMap[K_SHIFT] ? 10 : 5;
    if(keyMap[K_LEFT]){
        if(!keyMap[K_CTRL]) {
            if(camera.x > 0)
                camera.x -= speed;
        }
    }
    if(keyMap[K_RIGHT]){
        if(!keyMap[K_CTRL]){
            camera.x += speed;
        }
    }
    if(keyMap[K_DOWN]){
        if(!keyMap[K_CTRL]){
            camera.y += speed;
        }
    }
    if(keyMap[K_UP]){
        if(!keyMap[K_CTRL]){
            if(camera.y > 0)
                camera.y -= speed;
        }
    }
}

function selectTile(e) {
    let tileNum = parseInt(e.target.getAttribute("tileNum"));
    editor.tile = tileNum;
    editor.entity = null;
    let current = document.querySelector(".tile[selected=true]")
    if(current)
        current.removeAttribute("selected");
    e.target.setAttribute("selected", true);
}

function selectEntity(e) {
    let tileNum = parseInt(e.target.getAttribute("tileNum"));
    editor.tile = null
    editor.entity = tileNum;
    let current = document.querySelector(".tile[selected=true]")
    if(current)
        current.removeAttribute("selected");
    e.target.setAttribute("selected", true);
}

function newTile(e) {
    const i = document.createElement("input");
    i.setAttribute("type", "file");
    i.setAttribute("multiple", true);
    i.addEventListener("change", _e => {
        let files = i.files;
        let reader = new FileReader();
        let idx = 0;
        reader.onload = fe => {
            let tile = document.createElement("div");
            tile.classList.add("tile");
            tile.style.backgroundImage = `url(${fe.target.result})`;
            let tileNum = parseInt(files[idx].name.replace(".png", ""));
            let img = new Image();
            img.onload = _img => {
                console.log("loaded " + tileNum)
                editor.addTile(tileNum, _img.target);
            }
            img.src = fe.target.result;

            tile.addEventListener("click", selectTile);
            tile.setAttribute("tileNum", tileNum);
            e.target.parentNode.insertBefore(tile, e.target);
            if(idx < files.length - 1)
                reader.readAsDataURL(files[++idx])
        }
        reader.readAsDataURL(files[idx]);
    })
    i.click();
}

function newTileEntry(tileNum) {
    let tile = document.createElement("div");
    tile.classList.add("tile");
    tile.style.backgroundImage = `url(${tileNum}.png)`;
    let img = new Image();
    img.onload = _img => {
        editor.addTile(tileNum, _img.target);
    }
    img.src = `${tileNum}.png`;

    tile.addEventListener("click", selectTile);
    tile.setAttribute("tileNum", tileNum);
    document.querySelector("#tiles").insertBefore(tile, document.querySelector("#newTile"));
    
}

function newEntity(e) {
    const i = document.createElement("input");
    i.setAttribute("type", "file");
    i.setAttribute("multiple", true);
    i.addEventListener("change", _e => {
        let files = i.files;
        let reader = new FileReader();
        let idx = 0;
        reader.onload = fe => {
            let tile = document.createElement("div");
            tile.classList.add("tile");
            tile.style.backgroundImage = `url(${fe.target.result})`;
            let tileNum = parseInt(files[idx].name.replace(".png", ""));
            let img = new Image();
            img.onload = _img => {
                console.log("loaded! " + tileNum)
                console.log(_img);
                editor.addTile(tileNum, _img.target);
                
            }
            img.src = fe.target.result;

            tile.addEventListener("click", selectEntity);
            tile.setAttribute("tileNum", tileNum);
            e.target.parentNode.insertBefore(tile, e.target);
            if(idx < files.length - 1)
                reader.readAsDataURL(files[++idx])
        }
        reader.readAsDataURL(files[idx]);
    })
    i.click();
}

function newEntityTile(name, tileNum) {
    let tile = document.createElement("div");
    tile.classList.add("tile");
    tile.style.backgroundImage = `url(${name})`;
    let img = new Image();
    img.onload = _img => {
        console.log("loaded... " + tileNum)
        editor.addTile(tileNum, _img.target);
    }
    img.src = name;

    tile.addEventListener("click", selectEntity);
    tile.setAttribute("tileNum", tileNum);
    document.querySelector("#entities").insertBefore(tile, document.querySelector("#newEntity"));
}

function openEditor(tile) {
    const tileEditor = document.getElementById("tileeditor");
    tileEditor.setAttribute("tileX", tile.x);
    tileEditor.setAttribute("tile", tile.y);

    let title = document.createElement("h2");
    title.innerHTML = `Tile ${tile.x}, ${tile.y}`;

    let isDoor = document.createElement("input");
    isDoor.setAttribute("type", "checkbox");
    isDoor.setAttribute("id", "isDoor");
    if(tile.isDoor)
        isDoor.setAttribute("checked", true);

    let isDoorLabel = document.createElement("label");
    isDoorLabel.innerHTML += "Is Door? ";
    isDoorLabel.appendChild(isDoor);
    
    let keyType = document.createElement("input");
    keyType.setAttribute("type", "number");
    keyType.setAttribute("min", 0);
    keyType.setAttribute("max", 3);
    keyType.value = tile.key
    keyType.setAttribute("id", "keyType");

    let keyTypeLabel = document.createElement("label");
    keyTypeLabel.innerHTML = "Key type ";
    keyTypeLabel.append(keyType);

    let messageEdLabel = null;
    let messageEd = null;
    if(tile.wallTexture === 105 || tile.wallTexture == 115){
        messageEdLabel = document.createElement("label");
        messageEdLabel.innerHTML = "Sign Message ";
        messageEd = document.createElement("textarea");
        messageEd.innerHTML = tile.message || "";
        messageEd.setAttribute("id", "message");
    }

    let saveButton = document.createElement("button");
    saveButton.innerHTML = "Save";
    saveButton.addEventListener("click", () => saveTile(tile));

    tileEditor.innerHTML = "";
    tileEditor.appendChild(title);
    tileEditor.appendChild(document.createElement("hr"));
    tileEditor.appendChild(isDoorLabel);
    tileEditor.appendChild(document.createElement("hr"));
    tileEditor.appendChild(keyTypeLabel);
    tileEditor.appendChild(document.createElement("hr"));
    if(messageEdLabel)
        tileEditor.appendChild(messageEdLabel);
    if(messageEd)
        tileEditor.appendChild(messageEd);
        tileEditor.appendChild(document.createElement("hr"));
    tileEditor.appendChild(saveButton);
}

function saveTile(tile) {
    const isDoor = document.getElementById("isDoor");
    tile.isDoor = isDoor.checked;

    const keyType = document.getElementById("keyType");
    tile.key = keyType.value;

    if(tile.wallTexture === 105 || tile.wallTexture === 115){
        const message = document.getElementById("message").value;
        tile.message = message;
        console.log(message);
    }
}

function changeLayer(e) {
    editor.layer = parseInt(e.target.value);
}

function loadMap(e){
    var file = e.target.files[0];
    var reader = new FileReader();
    reader.onload = function(evt) {
        let buff = evt.target.result;
        let pad = (evt.target.result.byteLength % 2) !== 0;
        if(pad) {
            let resultView = new Uint8Array(evt.target.result);
            let _buff = new ArrayBuffer(evt.target.result.byteLength + 1);
            let _buffView = new Uint8Array(_buff);
            for(let i = 0; i < buff.byteLength; i++){
                _buffView[i] = resultView[i];
            }
            buff = _buff;
        }
        var byteArray = new Uint16Array(buff);
        parseMap(byteArray, buff);
    }
    reader.onerror = function(evt){
        console.error("Failed to read file! " + evt.type);
    }
    reader.readAsArrayBuffer(file);
}

function parseMap(map, rawBuffer){
    xMax = map[1];
    yMax = map[2];

    player.x = map[3];
    player.y = map[4];
    player.lookDir = map[5];

    parseWalls(map, rawBuffer);
    parseEntities(map, rawBuffer);
}

function parseWalls(map, rawBuffer) {
    for(let i = 0; i < xMax; i++) {
        for(let j = 0; j < yMax; j++) {
            let idx = 0;
            let offset = 6 + (i + (j * xMax)) * 8; // 8 shorts = 16 bytes
            let t = new Tile(i, j);
            t.wallTexture       = map[offset + idx++];
            t.ceilingTexture    = map[offset + idx++];
            t.floorTexture      = map[offset + idx++];
            if(t.wallTexture == 0 && t.ceilingTexture == 0 && t.floorTexture == 0)
                continue;
            t.zone              = (map[offset + idx] & 0xff00) >> 8;
            t.isDoor            = (map[offset + idx] & 1) != 0;
            t.key               = (map[offset + idx++] & 0b110) >> 1;
            if(t.wallTexture === 105 || t.wallTexture === 115) {
                let addrOffset = parseInt(((offset + idx) * 2));
                let addrArray = new Uint8Array(rawBuffer);
                let strOff = 0;
                strOff |= addrArray[addrOffset];
                strOff |= addrArray[addrOffset + 1] << 8;
                strOff |= addrArray[addrOffset + 2] << 16;
                strOff |= addrArray[addrOffset + 3] << 24;
                strOff |= addrArray[addrOffset + 4] << 32;
                strOff |= addrArray[addrOffset + 5] << 40;
                strOff |= addrArray[addrOffset + 6] << 48;
                strOff |= addrArray[addrOffset + 7] << 56;
                if(strOff === 0)
                    continue;
                t.message = buildString(addrArray, strOff);
            }
            
            grid[i][j] = t;
        }
    }
}

function parseEntities(map, rawBuffer) {
    let entLocation = 6 + 8 * xMax * yMax // 8 shorts = 16 bytes
    totalEnts = map[entLocation];
    for(let i = 0; i < totalEnts; i++) {
        let offset = 1 + i * 3; // 3 shorts: ent ID, x, and y pos
        let entNum = map[entLocation + offset++]
        let x = map[entLocation + offset++];
        let y = map[entLocation + offset++];
        entityGrid[x][y] = entNum;
    }
}

function buildString(byteArray, offset) {
    let str = "";
    let c = byteArray[offset];
    while(c) {
        str += String.fromCharCode(c);
        c = byteArray[++offset];
    }
    return str;
}

function parseEnts(map, rawBuffer) {

}

function saveMap() {
    let bin = new Uint16Array(new ArrayBuffer(xMax * yMax * 16 + 12 + totalEnts * 6 + 2)); //10 = 8 bytes for tiles, 10 bytes for entities. 12 = header information,totalEnts = total number  of ents * 2
    let idx = 0;

    bin[idx++] = editor._tiles.length;
    bin[idx++] = xMax;
    bin[idx++] = yMax;
    bin[idx++] = player.x;
    bin[idx++] = player.y;
    bin[idx++] = player.lookDir;

    let stringSetCallbacks = [];

    for(let i = 0; i < yMax; i++) {
        for(let j = 0; j < xMax; j++) {
            let wall = grid[j][i];
            if(!wall)
                wall = new Tile(i, j);
                //wall = new Tile(i, j);
            bin[idx++] = wall.wallTexture;
            bin[idx++] = wall.ceilingTexture;
            bin[idx++] = wall.floorTexture;
            bin[idx++] = (wall.zone << 8) | ((wall.isDoor ? 1 : 0) | (wall.key << 1));
            if(wall.wallTexture === 105 || wall.wallTexture === 115){
                stringSetCallbacks.push({
                    idx: idx,
                    wall: wall
                });
            }
            bin[idx++] = 0; // 8 blank bytes
            bin[idx++] = 0;
            bin[idx++] = 0;
            bin[idx++] = 0;
        }
    }
    
    bin[idx++] = totalEnts;
    for(let i = 0; i < yMax; i++){
        for(let j = 0; j < xMax; j++) {
            let e = entityGrid[j][i] ?? 0;
            if(!e)
                continue;
            console.log(`${i}, ${j} -> ${e} at ${idx}`);
            bin[idx++] = e;
            bin[idx++] = j;
            bin[idx++] = i;
        }
    }
    let firstLength = bin.byteLength;
    let strLength = stringSetCallbacks.length;
    for(let i of stringSetCallbacks){
        let str = i.wall.message ?? "";
        strLength += str.length;
    }
    let strbin = new Uint8Array(new ArrayBuffer(strLength));
    let sidx = 0;
    for(let scall of stringSetCallbacks){
        let str = scall.wall.message ?? "";
        const strOffset = sidx + firstLength;
        for(let c of str) {
            strbin[sidx++] = c.charCodeAt(0); 
        }
        strbin[sidx++] = 0; // null ended string
        bin[scall.idx + 3]  = (strOffset & 0xffff000000000000) >> 48;
        bin[scall.idx + 2]  = (strOffset & 0x0000ffff00000000) >> 32;
        bin[scall.idx + 1]  = (strOffset & 0x00000000ffff0000) >> 16;
        bin[scall.idx]      = (strOffset & 0x000000000000ffff);
    }

    let blob = new Blob([bin, strbin]);
    blob.lastModifiedDate = new Date();
    blob.name = "map.bin";

    var url = URL.createObjectURL(blob);
    var link = document.createElement("a");
    link.href = url;
    link.download = "map.bin";
    link.click();
}

function loadEntities() {
    //let xhr = new XMLHttpRequest();
    //xhr.onreadystatechange = function() {
    //    if(xhr.readyState == 4 && xhr.status == 200) {
            //tileDefs = JSON.parse(xhr.responseText);
            for(let i in entDefs) {
                newEntityTile("" +i, entDefs[i]);
            }
    //        console.log("Loading!")
            loadTiles();
    //    }
    //}
    //xhr.open("GET", "EntityDefs.json");
    //xhr.send();
    console.log("Loading");
}

function loadTiles() {
    //let xhr = new XMLHttpRequest();
    //xhr.onreadystatechange = function() {
    //    if(xhr.readyState == 4 && xhr.status == 200) {
            //tileDefs = JSON.parse(xhr.responseText);
            for(let i of tileDefs) {
                newTileEntry(i)
            }
            main();
    //    }
    //}
    //xhr.open("GET", "TileDefs.json");
    //xhr.send();
}

init();
loadEntities();
