#  Semantic Marker ™️ API Manual

![SemanticMarker API](SemanticMarkerAPI.png)

Creating and Processing with Semantic Marker™️ System is through various APIs (Application Programer Interface). This document will highlight the web calls using the http protocol and the javascript language. The following shows the interaction with the web services running at SemanticMarker.org
 
![SemanticMarker API Message](SemanticMarkerAPIMessages.png)

## SemanticMarker.js
This is javascript that will create a special Semantic Marker with the image Avatar inside.
<a href="SemanticMarker.js">SemanticMarker.js</a>

## Sample WEB javascript calls
This is called in the example javascript code at
<a href="sampleSMWeb.html">SampleSMWeb.html</a>

# SMART - Semantic Marker Augmented Reality of Things

The main interfaces to the SemanticMarker.org web services are through 
web calls embodied in the following web page code (and available at <a href="https://SemanticMarker.org/smart">SematnicMarker.org/smart</a>)

<a href="smart.html">smart.html</a>

## Semantic Marker javascript

The SMART web page makes calls through various javascript http calls. These are
embedded in the  <a href="smart.js">smart.js</a> code modules.

A typical <b>Http</b> web call might look like the following passing in a command which includes calls to the appropriate web interface (for example createSemanticMarker).

```javascript
//! sends the GET command, but shows an alert when done..
function createSemanticMarker(command)
{     
   var xhttp = new XMLHttpRequest();
   
   //!https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readystatechange_event
   xhttp.onreadystatechange = function() {
      // called when open is finished
      if (this.readyState == 4 && this.status == 200)
      {
      }
   };

  xhttp.open("GET", command, true);
  xhttp.send();
}  

```

