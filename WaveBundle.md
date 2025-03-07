# Format of the Knowledge Shark Wave Bundle

The `Knowledge Shark Wave Bundle` is a formated directory of artifacts.

A wave is a folder structure with images, multi-media artifacts (pdf, mov, mp4, etc). These are just
placed anywhere in the bundle and found dynamically. To get an order to the information a couple JSON formated meta-files are also included. 

When seen in the Semantic Marker&reg; app, the Knowledge Shark wave look as follows, where the user switches waves from this menu, or swiping down to get to the next stream.

<img src="images/KSWaveFromApp.png" width=300>

# Wave Bundle Structure
The structure of the bundle (which will be put into a `tar` file for later retrieval) is:

```
<APP.bundle name>
  MODEL.json 
  <multi media artifacts, mov, mp4, mp3>
  Stream1
     LIST.json
	  <image artifacts, png, jpeg, jpg>
  Stream1
     LIST.json
	  <image artifacts, png, jpeg, jpg>
  Stream3
     LIST.json
	  <image artifacts, png, jpeg, jpg>
```

An example can be found at the Knowledge Shark site:
<a href="https://KnowledgeShark.me/apps">Knowledge Shark Wave Bundles</a>

The following details the two `json` files.

# MODEL.json 
At the root level, this defines the wave name and the stream names. It also includes
any known Semantic Marker(s) and their known wave, stream, page. (Note the Semantic Marker(R) App
dynamically finds these values - and a version of the app exports these findings for inclusion in this MODEL.json file, reducing runtime constraints on what are slowly modified `Waves`.)

## Header

```json
  "class":"WavePhotoObjectsGeneric",
   "uuid": "uuid343aa243",
   "wave": "WaveB JITL",
```
## Streams

```json
 "streams" :
   [
      {
         "id": 1,
         "name": "stream1"
      },
      {
         "id": 2,
         "name": "stream2"
      },
      {
         "id": 3,
         "name": "stream3"
      }

   ],
```
## Related Waves

```json
 "relatedWaves" :
   [
       {
           "wave":"WaveD",
           "uuid":"34839849348"
       },
       {
           "wave":"WaveA",
           "uuid":"askdfjkasjdf"
       }
   ],
```

## Networking

```json
   "networking" :
   [
       {
           "key": "knowledgeShark",
           "commentMQTT": "defines the topic, user/password",
           "MQTT": "idogwatch/Dogs",
           "MQTT_USER": "USER",
           "MQTT_PASSWORD": "PASSWORD"
       }
   ],

```


## SemanticMarker


```json

    "SemanticMarker":
[{"sm":"https://tsdr.uspto.gov/documentviewer?caseId=sn97106637&docId=ORC20240324065012", "stream":"1", "page":"1"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685982547973", "stream":"2", "page":"1"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685983748003", "stream":"2", "page":"2"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685983778654", "stream":"2", "page":"3"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685983802087", "stream":"2", "page":"4"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685983823770", "stream":"2", "page":"5"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685983844420", "stream":"2", "page":"6"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685983862954", "stream":"2", "page":"7"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685228128240", "stream":"3", "page":"1"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685982257992", "stream":"3", "page":"2"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685982362041", "stream":"3", "page":"3"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685982428275", "stream":"3", "page":"4"},
{"sm":"https://SemanticMarker.org/bot/smart?uuid=QebkVLtEvPDI&flow=1685982492157", "stream":"3", "page":"5"}
]
```

## SemanticMarkers

```json
 "SemanticMarkers":
   [
       {"key":"Concurrency", "asset":"1.ConcurrencyICON"},
       {"key":"Discovery", "asset":"2.DiscoveryICON"},
       {"key":"Binding", "asset":"3.BindingICON"},
       {"key":"Protocols", "asset":"4.ProtocolsICON"},
       {"key":"Routing", "asset":"5.RoutingICON"},
       {"key":"Dissemination", "asset":"6.DisseminationICON"},
       {"key":"Operations", "asset":"7.OperationsICON"},
       {"key":"Challenge", "asset":"ChallengeICON"},
       {"key":"DeathStar", "asset":"DeathStarICON"},
       {"key":"ElevatorSpeech", "asset":"ElevatorSpeechICON"},
       {"key":"Iceberg", "asset":"IcebergICON"},
       {"key":"JITL", "asset":"JITLICON"},
       {"key":"KnowledgeShark", "asset":"KnowledgeSharkICON"},
       {"key":"Matrix", "asset":"MatrixICON"},
       {"key":"Stakeholder", "asset":"StakeholderICON"},
       {"key":"Story", "asset":"StoryICON"},
       {"key":"Language", "asset":"a.LanguageICON"},
       {"key":"Performance", "asset":"b.PerformanceICON"},
       {"key":"Security", "asset":"c.SecurityICON"},
       {"key":"Maintenance", "asset":"d.MaintenanceICON"},
       {"key":"Accessability", "asset":"e.AccessabilityICON"}
   ],

```

# Streams

Each of the 3 streams have a definition as well. This is inclued in the `LIST.json` file.

## LIST.json

```json
{
    "description": "stream1 description",
    "stream": "NDA Overview",
    "model":"3a,5c,5a,6e,7e",


    "images" :
    [
      "SemanticMarkerR.png",
      "SemanticMarkerRegisteredTrademark.png",
      "SMAppStore.jpg",
      "genres.jpg",
      "genre1.jpg",
      "genre2.jpg",
      "genre3.jpg",
      "genre4.jpg",
      "genre5.jpg",
      "JITL_Perspectives.jpg",
      "JITL_Views.jpg",
      "NDACube.jpg",
      "NDA_3d.jpg",
      "stakeholders.jpg",
      "SRP_Seam_icon.png",
      "SRP_image_icon.png",
      "SMMatrix2.png",
      "MatrixSparse.png"

    ]

}
```

> [!NOTE] 
> While in XCode, a template has been created that can be used to then fill in your own values.
>
> ![KSWave Template](images/KSWaveTemplate.png)

# Semantic Marker(R) Wave Address, or Knowledge Shark Wave, KSWave
## KSWave 

There needs to be a way to describe the URL of the wave. This is currently

```
https://KnowledgeShark.me/apps/<WaveName>.bundle.tar
```

- [ ] it could be extended on the URL to provide another wave name
```
https://KnowledgeShark.me/apps/<WaveName>.bundle.tar?wave=<OtherWaveName>
```


So this name can be added to the `KSWave` field in a SMART Button.

So when an address to the wave is handed out, the SMART Button will be used. The
question is whether there should be a URL to help the user know this is a wave?

Should it be:

```
https://SemanticMarker.org/bot/smart?uuid=x&flow=y&format=wave
```

or

```
https://SemanticMarker.org/bot/wave?uuid=x&flow=y
```

The receiver of a DOCFOLLOW with the later syntax will know this is a `KSWAVE` versus having to read the agent flow information,
other than the async access to the QRAvatar.

With `wave` there will have to be code that knows these, versus currently knowing about `smart`.

# Process KSWave
Here the URL is broken apart to get the wave name and optional other wave name.

- [ ] In Search, Scan or Matrix Clicking of KSWave objects, Ask user to load that wave (or no-op if already loaded), and that wave is rewinded..
- [ ] If DOCFOLLOW with `wave` syntax, the same can occur


# Seems the wave syntax is a good idea.

- [ ] look at simple demo.. using the KSWave of 
- [ ] only show

<b>(c) Kona Currents, LLC - v7.28.24
KSWave: v11.5.24
