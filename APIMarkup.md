
# Semantic Marker REST API 

This documents the main REST API calls to create and manage a Semantic Marker.
The current base URL for these is [SemanticMarker.org](https://SemanticMarker.org) but the entire set
can also be run locally or moved to other web servers. For the Semantic Marker to be sharable and run
by outside parties, the Semantic Marker address URL must be publically available (for example, http://localhost would only
be usable for local testing., but https://SemanticMarker.org is globally accessible)

Kona Currents, LLC offers three tiers of Semantic Marker offerings.

### Tier I. Semantic Marker Indirection 
### Tier II.  Semantic Marker Store and Forward
### Tier III.  Semantic Marker SMART Button Management

------------------------------------------------------------------------------------------
# Tier I Semantic Marker Indirection

A Tier I Semantic Marker is a straight indirection to a remote address. These are created 
directly from the various language implementation. Eamples of javascript, C and objective-c are shown. These
are snipits of code from various Semantic Marker apps to give a flavor of the APIs. 

<details>
<summary><code>Javascript</code> <code><b>new SemanticMarker</b></code></summary>

##### Example javascript
[Example HTML with javascript](sampleSMWeb.html)

Excerpt from html file:

```javascript
//The commandPath (the Semantic Marker address is a parameter as is the avatarURL, and if the 
avatar is circular or rectangle).
function createSemanticMarker(commandPath, avatarURL, circular)
{
try
   {
   // http web call to get the image from the AvatarURL
      getImage(avatarURL).then
      (
         function (successurl)
         {

            var sm = new SemanticMarker(document.getElementById(idSM),
               commandPath,
               avatarURL,
               circular);
         }).catch (function (errorurl)
         {

            alert("Issue creating Avatar in Semantic Marker&trade; creation: \n" + errorurl + "\nPlease file bug report");
         })

   }
   catch (error)
   {

      alert("Issue with Semantic Marker&trade; creation: \n" + error.message + "\nPlease file bug report");
   }
}
```

</details>


<details>
<summary><code>ESP-32 M5 Code</code> <code><b>create Semantic Marker</b></code></summary>

This is a simple example of creating a Tier I Semantic Marker inside the ESP-32 code base. 

##### Example ESP32 C Code
```c
   //!Draws a SemanticMarker&trade; 
    //!Create a SemanticMarker code with a width of 135 QR code  at (0, 100).  在(0,0)处创建一个宽为135二维码
    //!Please select the appropriate QR code version according to the number of characters.  请根据字符数量选择合适的二维码版本
    createSemanticMarker("https://iDogWatch.com/bot/help",0,100,135);
    //try to draw a character in the middle ...
    printTextAtTextPosition("SM",_zoomedTextPositions[QRAVATAR_ACTION]);
```
</details>

<details>
<summary><code>Apple iOS Objective-C</code> <code><b>create Semantic Marker</b></code></summary>
This is a simple example of creating a Tier I Semantic Marker inside the iOS Apple Objective-c code base. 

##### Example iOS ObjectiveC 
```objectiveC
    //! create new image with the Avatar Logo inside
    UIImage *image = [self createSemanticMarker:address avatarImage:image];
    
```
</details>

------------------------------------------------------------------------------------------
# Tier II Semantic Marker Store and Forward

A Tier II Semantic Marker invoves storing information at one of our Semantic Marker cloud servers. This
include an indirect address that a valid user can modify. Later, the Tier III Semantic Marker provides 
full user control.

## Creates a Tier II Semantic Marker using the Semantic Marker Address (escaped)

### Exists - called to see if this Semantic Marker has already been created
<details>
 <summary><code>GET</code> <code><b>/exists/ks/{category}/{namespace}/{UUID}/{escapedSemanticMarker}</b></code></summary>

#### Parameters

> | name      |  type     | data type               | description                                                           |
> |-----------|-----------|-------------------------|-----------------------------------------------------------------------|
> | namespace |  required | string                  | Namespace of Semantic Marker                                          |
> | category  |  required | string                  | Namespace of Semantic Marker                                          |
> | UUID      |  required | string                  | UUID of the user                                                      |
> | escapedAddress      |  required | string                  | The Semantic Marker Address is escaped so it is a single parameter argument|


#### Responses

> | http code     | content-type                      | response                                                            |
> |---------------|-----------------------------------|---------------------------------------------------------------------|
> | `201`         | `text/plain;charset=UTF-8`        | `Configuration created successfully`                                |
> | `400`         | `application/json`                | `{"code":"400","message":"Bad Request"}`                            |

#### Example cURL

> ```javascript
> set fullsm = "http://localhost:1880/exists/ks"
> curl -v  -F username=$user -F password=$pass -F link=$link -F kind=$kind $fullsm
> ```

</details>

### Assuming the Semantic Marker is unique, a new one is created (or trained)

### Train a semantic marker
<details>
 <summary><code>GET</code> <code><b>/train/ks/{namespace}/{category}/{UUID}/{escapedSemanticMarker}</b></code></summary>

##### Parameters

> | name      |  type     | data type               | description                                                           |
> |-----------|-----------|-------------------------|-----------------------------------------------------------------------|
> | namespace |  required | string                  | Namespace of Semantic Marker                                          |
> | category  |  required | string                  | Namespace of Semantic Marker                                          |
> | UUID      |  required | string                  | UUID of the user                                                      |
> | escapedAddress      |  required | string                  | The Semantic Marker Address is escaped so it is a single parameter argument|


##### Responses

> | http code     | content-type                      | response                                                            |
> |---------------|-----------------------------------|---------------------------------------------------------------------|
> | `201`         | `text/plain;charset=UTF-8`        | `Configuration created successfully`                                |
> | `400`         | `application/json`                | `{"code":"400","message":"Bad Request"}`                            |

##### Example cURL

> ```javascript
> set fullsm = "http://localhost:1880/train/ks"
> curl -v  -F username=$user -F password=$pass -F link=$link -F kind=$kind $fullsm
> ```

</details>

------------------------------------------------------------------------------------------
# Tier III Semantic Marker - SMART Buttons

A Tier III Semantic Marker provides storage for the Avatar image and for very fine control of 
Semantic Marker actions. The created result is called a SMART Button - standing for Semantic Marker Augmented Reality of Things. 

A SMART Button has a unique Semantic Marker address such as:

##### https://SemanticMarker.org/bot/smart?uuid=UUID&flow=flowNUM

#### Creating a Semantic Marker with an embedded Avatar Image
##### Digitize an image for use in a Semantic Marker Avatar (Note should have been cropped by the user)
<details>
 <summary><code>POST</code> <code><b>/{namespace}/{category}/{UUID}/digitize</b></code> </summary>

##### Parameters

> | name      |  type     | data type               | description                                                           |
> |-----------|-----------|-------------------------|-----------------------------------------------------------------------|
> | namespace |  required | string                  | Namespace of Semantic Marker                                          |
> | category  |  required | string                  | Namespace of Semantic Marker                                          |
> | UUID      |  required | string                  | UUID of the user                                                      |

##### Responses

> | http code     | content-type                      | response                                                            |
> |---------------|-----------------------------------|---------------------------------------------------------------------|
> | `201`         | `text/plain;charset=UTF-8`        | `http URL of resulting Avatar Image`                                |
> | `400`         | `application/json`                | `{"code":"400","message":"Bad Request"}`                            |

##### Payload
###### multipart/form-data

1.  Image file to be digitized

2. JSON of credentials

Like many of these interfaces, the credentials including the username and password, are passed as part of the payload in these POST calls.
```json
{ 
                         "username":_username,
                         "password":_password,
                         "name" : flowName,
                         "uuid" : uuid,
                         "flowNum"  : flowNum,
                         "flowCat"  : flowCat
                       }
```
##### Example cURL POST

> ```shell
> set fullsm = "https://SemanticMarker.org/bot/sm/$name/$cat/$uuid"
> curl --trace-ascii curl.trace  \
> -F filename=$filename \
> -F upload=@$filename \
> -F username=$user \
> -F password=$pass \
> -F link=$link \
> -F uuid=$uuid \
> -F flownum=$flownum \
> -F flowcat=$flowcat \
> -F kind=$kind \
> $fullsm

> ```

</details>


##### Create a SMART Button for an existing user of the Semantic Marker system
<details>
 <summary><code>POST</code> <code><b>/smflowpost</b></code> </summary>

##### Parameters

> | name      |  type     | data type               | description                                                           |
> |-----------|-----------|-------------------------|-----------------------------------------------------------------------|


##### Responses

> | http code     | content-type                      | response                                                            |
> |---------------|-----------------------------------|---------------------------------------------------------------------|
> | `201`         | `text/plain;charset=UTF-8`        | `http URL of resulting Avatar Image`                                |
> | `400`         | `application/json`                | `{"code":"400","message":"Your SemanticMarker Address was already registered."}`                            |

##### Payload
###### multipart/form-data

1. JSON of credentials for the SMART Button

Like many of these interfaces, the credentials including the username and password, are passed as part of the payload in these POST calls.
```json
{ 
                         "username":_username,
                         "password":_password,
                         "name" : flowName,
                         "uuid" : uuid,
                         "isdata": isDataFlag,
                         "private": privateflag,
                         "flowNum"  : flowNum,
                         "flowCat"  : flowCat,
                         "date" : dateString,
                         "parameters" : parameters,
                         "desc": desc,
                         "QRAvatarURL":QRAvatarURL,
                         "nextFlowURL":nextFlowURL,
                         "flow" : allCommandsJSON,

                         "dataSM" : dataSM,
                         "KSMatrix" : KSMatrixText,
                         "artifactsSM" : artifactsSM,
                         "markup" : markupText,
                         "languagesSM": languagesSM,
                         "videoSM": videoSM,
                         "isCircularAvatar": isCircularAvatar,

                         "KSWave" : KSWaveText,
                         "bridgeSM" : bridgeSM,
                         "future" : futureText,

                         "audioSM" : audioSM,
                         "locationSM" : locationSM,
                         "inheritedSM" : inheritedSM,
                         "deckSM" : deckSM,
                         "isMessaging" : isMessaging,
                         "isIndirectSM" : isIndirectSM,
                         "indirectSM" : indirectSM
                       }

                       
```
##### Example cURL POST

> ```shell
> set fullsm = "https://SemanticMarker.org/bot/smflowpost"
> curl --trace-ascii curl.trace  \
> $fullsm

> ```

</details>
------------------------------------------------------------------------------------------

# Additional Interaction with the SMART Button Infrastructure

Outside of creating a Semantic Marker, modification and invocation is out of scope of this API document. One
web page that encapsulates most of these interactions can seen at:  [SemanticMarker.org/smart](https://SemanticMarker.org/smart)
