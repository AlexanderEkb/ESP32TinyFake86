# Fake86
Port of the Fake86 emulator (rpsubc8 and Mike Chambers) to the ESP32-WROVER module using advantages of PSRAM, SD-Card and so on.
<br>
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/pakupaku.gif'></center>
I have made several modifications:
<ul>
 <li>PSRAM used, so there is a full amount of 640 KB base memory available in emulated machine</li> 
 <li>VGA video is replaced by the NTSC video output borrowed from ESP32 Dali Clock project</li>
 <li>No disk images are precompiled together with the project. SD-Card is used to store them. It's possible to switch images on the fly</li>
 <li>To make the whole thing more compact, a custom keyboard is used instead of PS2 one. So its driver is also rewritten.</li>
</ul> 


<br><br>
<h1>Requirements</h1>
Required:
 <ul>
  <li>ESP32-WROVER module with PSRAM</li>
  <li>VSCode and PLATFORMIO</li>
 </ul>
<br>


<br><br>
<h1>PlatformIO</h1>
PLATFORMIO must be installed from the Visual Studio extensions.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewPlatformIOinstall.gif'></center>
Then select the working directory <b>Tinyfake86ttgovga32</b>.
We must modify the file <b>platformio.ini</b> the option <b>upload_port</b> to select the COM port where we have our TTGO VGA32 board.
<center><img src='https://raw.githubusercontent.com/rpsubc8/ESP32TinyFake86/main/preview/previewPlatformIO.gif'></center>
Then we will proceed to compile and upload to the board. No partitions are used, so we must upload the entire compiled binary.
It's all set up so we don't have to install any libraries.


<br><br>
<h1>Arduino IDE</h1>
I never did attempt to build this project in Arduino IDE, sorry.
