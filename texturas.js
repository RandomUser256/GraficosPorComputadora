////-------------------------
//// Cambiar la imagen de textura usando var teximage
////
////
////-------------------------

var ww = window.innerWidth,
  wh = window.innerHeight;

function init() {

  renderer = new THREE.WebGLRenderer({
    canvas: document.getElementById('scene')
  });
  renderer.setClearColor(0xFFFFFF);
  renderer.setSize(ww, wh);

  scene = new THREE.Scene();
  camera = new THREE.PerspectiveCamera(50, ww / wh, 1, 10000);
  camera.position.set(0, 0, 700);
  scene.add(camera);

  light = new THREE.DirectionalLight(0xffffff, 1);
  light.position.set(0, 250, 700);
  scene.add(light);

  createBox();

  renderer.render(scene, camera);
  previousTime = Date.now();
  animate();
};

function createBox() {

  elements = new THREE.Object3D();

  //Create a geometry for a cube
  geometry = new THREE.BoxGeometry(200, 200, 200);

  //Create a Texture Loader object
  var loader = new THREE.TextureLoader();
  loader.crossOrigin = 'anonymous';
  var texture = loader.load(texImage);


  ////-------------------------
  //// Crear Box #1 cambiando la  
  //// imagen de textura.  
  //// Incluir iluminacion difusa
  ////-------------------------


  //Add it into our group
  elements.add(box1);

  ////-------------------------
  //// Crear Box #2 agregando color 
  //// en la función de textura
  ////-------------------------
 


  ////-------------------------
  //// Crear Box #3 utilizando un
  //// modelo de iluminación diferente 
  //// al material difuso (ej. Phong)
  ////------------------------- 


  scene.add(elements);

};


var counter = 0;
var animate = function(a) {

  requestAnimationFrame(animate);
  box1.rotation.y += .01;


  var frameTime = Date.now();
  var delta = frameTime - previousTime;
  if (delta > 50) {
    previousTime = frameTime;
    sprite.offset.x = (counter % 4) / 4;
    sprite.offset.y = Math.floor(counter / 4) / 3;
    counter++;
    if (counter == 12) {
      counter = 0;
    }
  }

  renderer.render(scene, camera);
};

init();
