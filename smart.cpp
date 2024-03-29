#ifdef INCLUDE_HERE
//NOTE: splitting the iDogWatch.com from SemanticMarker.org .. 
//  6.11.23 the UserDB that node-red reads from is a COPY of the master if 
//  run seperately (split).

//! this is the address things use for later 
var _semanticMarkerFlowURL = "https://SemanticMarker.org/bot/";

// this is before moving to the new smart deck..
var _useOldSMARTDeck = true;

//var _semanticMarkerFlowURL = "http://localhost:1880/";
//var _semanticMarkerFlowURL = "http://localhost:2001/";

//! address of the rootBotURL (iDogwatch.com right now)
//! THis is the address of the node-red we are calling..

var _rootBotURL = "https://iDogWatch.com/bot";
//var _rootBotURL = "http://localhost:1880";


// BELOW: defines var _rootBotURL = "https://iDogWatch.com/bot";
//   These are all the "messages"   vs localhost:1880 
// NOTE: these can modify the agent (but are done by the AWS instance)
//NOTE: 'addUser' and 'adddevice' could be sent to the SemanticMarker.org bot as well (a duplicate, or at least
// and in-sync version  (versus a duplicate)

// Change to TRUE on the final version.. AND uncomment out the _devicesJSON (since the JSON cannot use a " "
//let _useFinal = true;
let _useFinal = false;
var _username = null;
var _password = null;
// _word is used for the guest password .. I think it was a worry the cookie was goofy..
// 12.11.23 .. put back _guestPassword
var _guestPassword;
var _saveUserInfo;
var _devicesJSON =  [{"deviceName":"_GENERIC_"},{"deviceName":"ScoobyDoo"},{"deviceName":"PumpkinUno"},{"deviceName":"GreyGoose"},{"deviceName":"DukeGEN3"},{"deviceName":"HowieFeeder"}, {"deviceName":"M55"}, {"deviceName":"M5Rainier"}, {"deviceName":"LauraBabe"}, {"deviceName":"M522"}, {"deviceName":"M5Rainier"}, {"deviceName":"M5WRR"},{"deviceName":"MoonShot"},{"deviceName":"LacieUSBC"}];

//Array of: [uuid, deckname, deckcat, deckpassword, date]
var _smartDecks = [];

// 10.31.23
//Array of: [gpassword: pass]
var _guestPasswordsJSON = [];

//! initialize when refreshing after logoff
function initGlobals()
{
  _smartDecks = [];
  _devicesJSON = [];
  _username = "";
  _password = "";
  _guestPassword = "";
  _UUIDNumbers = [];
  _guestPasswordsJSON = [];

  //clear out  the SMART Deck
  document.getElementById('_UUID_Select').innerHTML = "";
  document.getElementById('flowsSM').innerHTML = "";
}

// checks username password have values ..
function validUserInfo()
{
   if (_username && _username.length > 0 && 
	    _password && _password.length > 0)
		 return true;
	else 
	    return false;

}

//NO _useFinal
if (_useFinal)
{
   _username = "{{req.params.NAME}}";
   _password = "{{req.params.PASSWORD}}";
   _guestPassword = "{{req.params.guestPassword}}";
	// userInfo == "super" or "normal"
   _saveUserInfo = "{{req.params.userinfo}}";

	// ***** manually uncommend out the following.. ****
    //_devicesJSON = {{{payload.devicenames}}};
	 //_smartDecks = {{{payload.smartDecksJSON}}};
}
else
{

    //_username = "scott@konacurrents.com";
    //_password = "pass";
    //_username = "test";
    //_password = "test";

    _guestPassword = "NOT_SET";
     _devicesJSON =  [{"deviceName":"_GENERIC_"},{"deviceName":"ScoobyDoo"},{"deviceName":"PumpkinUno"},{"deviceName":"GreyGoose"},{"deviceName":"DukeGEN3"},{"deviceName":"HowieFeeder"}, {"deviceName":"M55"}, {"deviceName":"M5Rainier"}, {"deviceName":"LauraBabe"}, {"deviceName":"M522"}, {"deviceName":"M5Rainier"}, {"deviceName":"M5WRR"},{"deviceName":"MoonShot"},{"deviceName":"LacieUSBC"}];

	  //_smartDecks = [{"uuid":"OvdEK4Ks_j2Z","deckname":"marker1","deckcat":"LegacyMarkers","deckpassword":"darwin","date":1679955654862}];

}

//TODO: make it like the _flowJSON
var _UUIDNumbers = [];

// modified by Base64 or Pulldown
var _deviceName;
var _isSendToAll;
// This has to be in sync with mainModule.h
var _lastSMPage = "sm14";

//! variables (fixed right now) for the Semantic Marker parameters
//!@see https://github.com/konacurrents/KSQRAvatar/issues/54
//! This is returned when creating a SM user..
var _SemanticMarker_UUID = "soIZ1iH7FK";

// currently these are hardwired and must be lock step ([0] == username, and [0] == _username)
// eg.  [{"USERNAME":_username},{"PASSWORD",_password}]
var _SemanticMarkerParameters = ["USERNAME","PASSWORD","GUESTPASSWORD"];
// on startup these are unknown..
var _SemanticMarkerValues = [_username, _password, _guestPassword];

//! called to set the values .. after they are known
function setSemanticMarkerUserPasswordValues()
{
  console.log("setSemanticMarkerUserPasswordValues: " + _username + ", " + _password + "," + _guestPassword);
  _SemanticMarkerValues = [_username, _password, _guestPassword];
}

//! the delay in seconds
var _commandDelay;
//! last run number
var _nextRunCommand;
//! count of commands
var _maxRunCommands;

//! logout .. erases cookies
function logout()
{
    alert("This logs you out so next time you have to enter password and optionally a different username");
	 _username = "";
    _password = "";
	 saveCookies();
	// requestLoginWindow(true);

	document.getElementById("_Login").hidden = false;

	//initGlobals();

	// do a full page reload .. hopefully that clears the screen
   location.reload();
	//initWebPage();
}

//! login
function login()
{
   //requestLoginWindow(true);
   location.reload();
}

//! try a login. 
//! Cannot hide password yet: https://stackoverflow.com/questions/9554987/how-can-i-hide-the-password-entered-via-a-javascript-dialog-prompt
function requestLoginWindow(doReadCookies)
{
	if (doReadCookies)
	  readCookies();

	// basically is password is null then ask for both.. but reuse the username
	// but if username null do it too
   if (!validUserInfo())
	{
      let username = prompt("Please enter your username: ",_username);
	   _username = username;

	   let password = prompt("Please enter password","");
	   _password = password;
	}

	saveCookies();

   reloadUserAgent();

}

/* ******************************  End Globals ****************** */

// 3.27.23
// update the _UUID_Select with the options..
//   uses global _smartDecks  so that needs to be updated 
//   on a refresh on new SmartDeck and on "Get User SM Flows"
function parseSmartDecks(agentUUID)
{
      // update the selection ..
	   // _UUID_Select
	   var uuidSelect = document.getElementById('_UUUID_Select');
   
	   // Category
	   //    Name (UUID)
	   //    Name2 (UUID)
	   // Category 

		//!Try sorting by Category, then Num
		//! https://stackoverflow.com/questions/4833651/javascript-array-sort-and-unique
		//! First find the categories..
		var deckCategories = []; //deckCat
		var uuidNumbers = []; //UUID
		var deckNames = []; 

		// create a new smartDecks with the main agent included..
		var allDecks = _smartDecks;
		var mainAgentCategory = "Main Agent";
    
    // this is created already in the conversion to SMUsers DB
    if (_useOldSMARTDeck)
    {
        var alreadyAdded = false;
        // see if already done...
        for (var i=0; i< allDecks.length; i++)
        {
            if (allDecks[i].uuid == agentUUID)
            {
                alreadyAdded = true;
                break;
            }
        }
        
        if (!alreadyAdded)
        {
            
            // add main deck.. at the front..
            allDecks.push(
                          {
                              "uuid":agentUUID,
                              "deckcat": mainAgentCategory,
                              "deckname":"Root"
                          });
        }
    }

		// the main agent isn't in the smart decks .. so add it..
	   for (var i=0; i< allDecks.length;i++)
	   {
	      var deck = allDecks[i];
		   var uuid = deck.uuid;
			var deckCategory = deck.deckcat;
			var deckName = deck.deckname;
			deckCategories.push(deckCategory);
			uuidNumbers.push(uuid);
			deckNames.push(deckName);
		}
		//!alphebet sort
		deckCategories.sort();

		//!alphebet sort
		deckNames.sort();

		//! sort numbers so 10 and 100 are in right order (Actually not used as the NAME is the sort)
		//! https://stackoverflow.com/questions/1063007/how-to-sort-an-array-of-integers-correctly
		uuidNumbers.sort(function(a,b) { return a-b; });

		// now make them unique (new Set) and back to an array
		deckCategories = Array.from(new Set(deckCategories));

		//sort
		deckCategories.sort();

		//how to make the Main one first?? TODO..
		// Will remove it.. then add to the front..
		var mainIndex = deckCategories.indexOf(mainAgentCategory);

		// remove
		deckCategories.splice(mainIndex, 1);

		// add to front 
		deckCategories.unshift(mainAgentCategory);
		
		uuidNumbers = Array.from(new Set(uuidNumbers));
		deckNames = Array.from(new Set(deckNames));


      flowsHTML = "";

		//! groups: https://stackoverflow.com/questions/17316540/make-some-options-in-a-select-menu-unselectable
		for (deckCategory of deckCategories)
		{
			flowsHTML += "<optgroup label='" + deckCategory + "'>" + "\n";

			for (deckName of deckNames)
			{
            for (var i =0; i< allDecks.length; i++)
		      {
			      var deck = allDecks[i];
					thisUUID = deck.uuid;
					thisDeckCat = deck.deckcat;
					thisDeckName = deck.deckname;

					// only continue if same deck category
					if (thisDeckCat.localeCompare(deckCategory) != 0)
					   continue;

					// only continue if same deck name
					if (thisDeckName.localeCompare(deckName) != 0)
					   continue;

					// now we are the correct deckCategory, and deckName
		         for (uuidNum of uuidNumbers)
				   {
						if (!thisUUID)
						   continue;

					   if (thisUUID.localeCompare(uuidNum) != 0)
						   continue;

						// Finally output an option  (name:UUID)
			         flowsHTML += "<option value='" + thisUUID + "'>" + thisDeckName + " (" + uuidNum + ")" + "</option>\n";
   
				   }
				}

			}
			flowsHTML += "</optgroup> \n";
		}

		//update the html select _UUID_Select
	   document.getElementById('_UUID_Select').innerHTML = flowsHTML;


		//5.6.23 
		// Store the _UUID's of this Agent. Then if we traverse to another Smart DECK
		// From a different user (such as through inheritance), it won't think is
		// can save (without cloning).
		// NOTE: if we clone, then it would be nice to fix the parent - to now point to this one..

		_UUIDNumbers = uuidNumbers;
}

// 2048 characters on the URL max..
//!invoke the urlID
function invokeURL(url)
{
	window.open(url);
}

//!invoke the urlID
function invokeURLid(urlID)
{

   var url = document.getElementById(urlID).value;
	//! use the web format
	url += "&format=web";
	window.open(url);
	//callGetCommand(url);
}
/* ******** START NOT USED 

//TODO.. get the RUN SemanticMarkers working...
//! make web get call
function callGetCommand(url)
{
   var xhttp = new XMLHttpRequest();

	//!https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readystatechange_event
   xhttp.onreadystatechange = function() {
		// called when open is finished
      if (this.readyState == 4 && this.status == 200)
	   {
      }
   };
   //! shows the command at this element
   document.getElementById('_commandRunText').value = url;

   xhttp.open("GET", url, true);
   xhttp.send();
}

//! array of links for the flow
var _finalLinkArray;

//!runSemanticMarkers (recursive after the next delay) until max is hit
function runSemanticMarkers()
{
   _commandDelay = document.getElementById('_delayAmountId').value;

	if (_nextRunCommand >= _maxRunCommands)
	{
		alert("Finished running commands");
		_nextRunCommand = 0;
	   return;
	}

   // grab the next run command (from the select)
	var command = _finalLinkArray[_nextRunCommand];
	_nextRunCommand++;

	// call it..
   callGetCommand(command);

	//recurse
	var delayInMilliseconds = _commandDelay * 1000;
	setTimeout(function()
	{
	   runSemanticMarkers();
	}, delayInMilliseconds);
}


//!runSemanticMarker
//@param linkId == <select option array>
function runSemanticMarker(linkId)
{
   // this uses the parameters .. rep
	//var command = document.getElementById(linkId).value;
   //callGetCommand(command);

	//set default
	_nextRunCommand = 0;

	 // the array of commands
    _finalLinkArray = [];

	 // commands = option array
	 var commands = document.getElementById(linkId);

	 //! foreach arg.. replace the Parameter with the equilivent instance (eg. password -> _password)
	 for (var i=0; i< commands.length; i++)
	 {
        var command = commands[i].innerHTML;
		  _finalLinkArray.push(command);
	 }

	//set max
	_maxRunCommands = _finalLinkArray.length;

	//
	runSemanticMarkers();
}

********* END  not used ********/

//!invoke the urlID, but tack on the extraURL
function invokeURLid2(urlID, extraURL)
{

   var url = document.getElementById(urlID).value;
	url = url + extraURL;
	window.open(url);
	//callGetCommand(url);
}

//! gets and saves UUID
//! global:  _uuidStringText
function updateSemanticMarkerUUID()
{
  var uuid = document.getElementById('_uuidStringText').value;
   _SemanticMarker_UUID = uuid;
}

//! strips spaces, but supports numbers letters and _ 
function createValidDeviceName(deviceName)
{

	// replace NOT of these  NOTE: "_" is NOW allowed.  9.5.23
   var name = deviceName.replace(/[^a-zA-Z0-9_]/g, "");
	return name;
}

//!strip spaces (THis is wrong.. it only removes the start/end spaces
function stripSpaces(cmd)
{
   //cmd = cmd.trim();
	//! https://stackoverflow.com/questions/5963182/how-to-remove-spaces-from-a-string-using-javascript
	cmd = cmd.replace(/\s/g, '');
	return cmd;

}

//! strips spaces, but supports numbers letters and _ 
function createValidFlowName(flowName)
{

	// replace NOT of these 
   var name = flowName.replace(/[^a-zA-Z0-9_]/g, "");
	return name;
}

//! changes a string to replace spaces with %20
function escapeSpacesInString(string)
{
	// need to escape the &  (as far as we know)
   //string = string.replace(/&/g,"\\&");
   string = string.replace(/ /g,"%20");
	return string;
}


//! changes a string to replace spaces with %20
//! ISSUE: https://stackoverflow.com/questions/11449577/why-is-base64-encode-adding-a-slash-in-the-result
function escapeBase64InString(string)
{
  // When received, to atob(str)
  //  then str = decodeURIComponent(atob(str))

	// escape the URI first..
	var moreString2 = encodeURIComponent(string);
	// Then the btoa doesn't have "/" in it..
	var s2 = btoa(moreString2);
	return s2;

	// need to escape the &  (as far as we know)
   //string = string.replace(/&/g,"\\&");
	//var moreString = btoa(string);
   //string = moreString;
   //string = string.replace(/ /g,"%20");
	//return string;
}

// object = {"SM":sm,"img":<img src=... }
var _base64Images = [];

//! 5.9.23 call the getSM/UUID/FLOW  and when 
//! it returns, store the <img src=base64 ..
//! gets or creates a UUID for existing user "/createuuid"
function requestBase64Image(uuid, flowNum)
{
  let thisSM = createSM(uuid, flowNum);
  var command = _semanticMarkerFlowURL + "getSM/" + uuid + "/" + flowNum ;

  //! see if already cached..
  for (let i=0;i < _base64Images.length; i++)
  {
     if (thisSM.localeCompare(_base64Images[i].SM)==0)
	  {
	     console.log("Already grabbed _base64Image for: " + thisSM);
		  return;
	  }
  }

  // create placeholder
  var object = {"SM":thisSM,"img":null};

  //add to the list being requested
  _base64Images.push(object);

  var xhttp = new XMLHttpRequest();

  // update the wait count
  changeWaitingCount();

  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
		//!result is the USER object (not the SemanticMaker object)
		var jsonReply = this.responseText;

	//	console.log("reply: " + jsonReply);
		if (jsonReply && jsonReply.indexOf("error") >= 0)
		{

         alert(jsonReply);
			return;
		}
		if (jsonReply.length == 0 )
		{

         alert("You must login to your agent");
			return;
		}

       //! update the outstanding request for this image
       for (let i=0;i < _base64Images.length; i++)
       {
          if (thisSM.localeCompare(_base64Images[i].SM)==0)
	       {
		       _base64Images[i].img = jsonReply;
	       }
       }

       // update the wait count
       changeWaitingCount();

    }
  };
  xhttp.open("GET",command, true);
  xhttp.send();
}

//! gets the smartDecks of the user 6.13.23
// if (!_useOldSMARTDeck)  .. still working the transition (this isn't in SM.org yet)
function getSMSmartDecks(uuidStringTextId)
{
  var command = _semanticMarkerFlowURL + "getuserdecks/" + _username + "/" + _password ;

  var xhttp = new XMLHttpRequest();

  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
		//!result is the USER object (not the SemanticMaker object)
		var jsonReply = this.responseText;

		console.log("reply: " + jsonReply);
		if (jsonReply && jsonReply.indexOf("error") >= 0)
		{

         alert(jsonReply);
			return;
		}
		if (jsonReply.length == 0 )
		{

         alert("You must login to your agent");
			return;
		}

      var userJSON = JSON.parse(jsonReply);
		// grab array of smart decks
		//var smartDecks = userJSON.smartDecks;
		smartDecks = userJSON;
		_smartDecks = smartDecks;

		//var uuid = userJSON.uuid;
      //document.getElementById(uuidStringTextId).value = uuid;

	   // update the deck options  
      parseSmartDecks(null);
    }
  };
  xhttp.open("GET",command, true);
  xhttp.send();
}


//! gets or creates a UUID for existing user "/createuuid"
function getSMUserUUID(uuidStringTextId)
{
  if (!validUserInfo())
     return;

  var command = _semanticMarkerFlowURL + "createuuid/" + _username + "/" + _password ;

  var xhttp = new XMLHttpRequest();

  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
		//!result is the USER object (not the SemanticMaker object)
		var jsonReply = this.responseText;

		console.log("reply: " + jsonReply);


      var userJSON = JSON.parse(jsonReply);
		var uuid = userJSON.uuid;
      document.getElementById(uuidStringTextId).value = uuid;

	   // update the deck options  (Call from getSMUserUUID...
      parseSmartDecks(uuid);
    }
  };
  xhttp.open("GET",command, true);
  xhttp.send();
}

//!create the list of devices.. Fill in the idDeviceListOptions (which has the select already formed)
function createDeviceListOptions(idDeviceListOptions)
{

  var htmlSelect = "";
  htmlSelect += "<option value='NO_ONE'>NO_ONE</option>";
  htmlSelect += "\n";
  htmlSelect += "<option value='--ALL--'>--ALL--</option>";
  htmlSelect += "\n";

  // used for sorting later..
  var deviceNames = [];

  // go through list, but dont include GENERIC and DEFAULT
  for (var i =0; i< _devicesJSON.length; i++)
  {
		  //Not sorting yet..
        let deviceName = _devicesJSON[i].deviceName;
        if ("_GENERIC_".localeCompare(deviceName)!=0 && "_DEFAULT_".localeCompare(deviceName) != 0)
        {
			 deviceNames.push(deviceName);
		  }
	}

	//now sort
	deviceNames.sort();
   for (var i =0; i< deviceNames.length; i++)
   {
     let deviceName = deviceNames[i];

	  // create option for the deviceName
     htmlSelect += "<option value='" + deviceName + "'>" + deviceName + "</option>";
     htmlSelect += "\n";
   }

  document.getElementById(idDeviceListOptions).innerHTML = htmlSelect;
}

//! 10.31.23 
//!create the list of guest passwords.. Fill in the guestPasswordsSelectID (which has the select already formed)
function createGuestPasswordListOptions(guestPasswordsSelectID)
{

  var htmlSelect = "";
  htmlSelect += "<option value='NONE'>NONE</option>";
  htmlSelect += "\n";

  // used for sorting later..
  var guestPasswords = [];

  // go through list, but dont include GENERIC and DEFAULT
  for (var i =0; i< _guestPasswordsJSON.length; i++)
  {
		  //Not sorting yet..
        let guestPassword = _guestPasswordsJSON[i].gpassword;
		  guestPasswords.push(guestPassword);
	}

	//now sort
	guestPasswords.sort();
   for (var i =0; i< guestPasswords.length; i++)
   {
     let guestPassword = guestPasswords[i];

	  // create option for the deviceName
     htmlSelect += "<option value='" + guestPassword + "'>" + guestPassword + "</option>";
     htmlSelect += "\n";
   }

  document.getElementById(guestPasswordsSelectID).innerHTML = htmlSelect;
}

//! wraps addNewGuestPassword
function sendAddNewGuestPassword(idGuestPasswordString)
{
   var addPassword = document.getElementById(idGuestPasswordString).value;
	var values = [_username, _password, addPassword];
   sendURLValues("addguestpassword", values);
}

//! wraps removeGuestPassword
function sendRemoveGuestPassword(idGuestPasswordString)
{
   var removePassword = document.getElementById(idGuestPasswordString).value;
	var values = [_username, _password, removePassword];
   sendURLValues("removeguestpassword", values);
}

//! just sent all for FEED and CAPTURE, using send: feed
function sendAllCommand(command, cmd)
{
	_isSendToAll = true;
	var commandPath = getCommandPath(command);

   sendGETCommand(commandPath + "/" + cmd);
}

//! returns string up the argments..
//! sets up the device and isSendToAll
function setupDevice(idDevices)
{
   var deviceName = document.getElementById(idDevices).value;

	if ("--ALL--".localeCompare(deviceName)==0)
	   _isSendToAll = true;
	else
	   _isSendToAll = false;

	_deviceName = deviceName;
}

//! sets using the password provided, so can be user password or guest password
//! This uses knowledge of _isSentToAll, and _deviceName, and this command (which tacks on Device if needed)
function getCommandPathWithPassword(command,password)
{
  if (!validUserInfo())
     return;

  var commandPath = "";
  if (_isSendToAll)
  {
     commandPath = _rootBotURL + "/" + command + "/" + _username + "/" + password;
  }
  else
  {
	  //NOTE: lowercase all..
     command = command + "device";
     commandPath = _rootBotURL + "/" + command + "/" + _username + "/" + password + "/" +  _deviceName;
  }
  return commandPath;
}

//! Returns a SemanticMarker parameterized command string
//! This uses knowledge of _isSentToAll, and _deviceName, and this command (which tacks on Device if needed)
function getCommandPathWithSMParameters(command)
{
  var commandPath = "";
  if (_isSendToAll)
  {
     commandPath = _rootBotURL + "/" + command + "/" + _SemanticMarkerParameters[0] + "/" + _SemanticMarkerParameters[1];
  }
  else
  {
	  //NOTE: lowercase all..
     command = command + "device";
     commandPath = _rootBotURL + "/" + command + "/" + _SemanticMarkerParameters[0] + "/" + _SemanticMarkerParameters[1] + "/" +  _deviceName;
  }
  return commandPath;
}

//! This uses knowledge of _isSentToAll, and _deviceName, and this command (which tacks on Device if needed)
function getCommandPath(command)
{
  return getCommandPathWithPassword(command,_password);
}

//! This uses knowledge of _isSentToAll, and _deviceName, and this command (which tacks on Device if needed)
function getCommandPathGuest(command)
{
  // _word is the guest password
  // 11.21.23 .. if the _guestPasswordsSelectId is set, then use that one..
  // id="_guestPasswordsSelectID" class="select btnB" >x</select>
  var selectedGuestPassword = document.getElementById('_guestPasswordsSelectID').value;
  var command;
  if (selectedGuestPassword && selectedGuestPassword.length > 0 &&  selectedGuestPassword != "NONE")
    command = getCommandPathWithPassword(command,selectedGuestPassword);
  else
    command = getCommandPathWithPassword(command,_guestPassword);
  console.log("Guest command = '" + command + "'");
  return command;
}

//! wraps the command no args, but this is sent as a guest
function sendNoArgsGuest(idDevices, idCmd)
{
	// one way: but not doing that here..
	// using  feedguestdevice/U/P/device
   // https://iDogWatch.com/bot/feedguestdevices/scott@konacurrents.com/GPASS?device=ScoobyDoo
	// sets _devices
	setupDevice(idDevices);


	// the command, which we might change below based on _isSendToAll
   var command = document.getElementById(idCmd).value;

	//eg.  feedguest<device>/USER/GPASS/<device>
	//feedguest/name/guest   or  feedguestdevice/name/guest/device
   // this will use guest password..
	var commandPath = getCommandPathGuest(command);

  //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
  sendGETCommand(commandPath);
}

//! wraps the command no args
function sendNoArgs(idDevices, idCmd)
{
	setupDevice(idDevices);

	// the command. NOTE: there are some that use the "send" syntax still 5.4.23
	var cmd = document.getElementById(idCmd).value;
	var command = "cmd";
	if ("status".localeCompare(cmd)==0
	 || "temp".localeCompare(cmd)==0
	 || "capture".localeCompare(cmd)==0
	 || "volume".localeCompare(cmd)==0)
	   command = "send";

	var commandPath = getCommandPath(command);

	//! 1.13.24
	cmd += getCurrentGroupNameQuery(false);

	// bot/cmddevice/NAME/PASS/device/CMD

  //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
  sendGETCommand(commandPath + "/" + cmd);
}

//! wraps the command no args. Send "feed" or "capture" for example without arguments
function sendCommandToDevice(idDevices, cmd)
{
	setupDevice(idDevices);

	// the command
	var command = "cmd";
	var commandPath = getCommandPath(command);

  //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
  sendGETCommand(commandPath + "/" + cmd);
}

//! 12.17.23 return a base64 string from a JSON string
function createBase64FromJSON(stringJSON)
{
    var base64StringJSON  =  escapeBase64InString(stringJSON);
	 return base64StringJSON;
}

//! 12.17.23 create base64
function createBase64ForCredentials()
{
   var stringJSON = { "username" : _username,
	                   "password" : _password,
							 "guestPassword" : _guestPassword
							 };
   var base = createBase64FromJSON(JSON.stringify(stringJSON));

	return base;
}

//! Special case sending any set:val commands
//!  set:cmd, val:arg
//! This version converts the val into base64, and the command is added with 64
function sendAnySetCommand64Both(idDevices, anySetCommandId, anySetValId, encodeQuery)
{
	// the command (base) .. device is tacked on 
	var command = "set64";
	if (encodeQuery)
	{
		// the _device global is updated to a base64 value
		// The name has to escape the spaces and | and & 
	   var device = document.getElementById(idDevices).value;

      device = device.replace(/&/g,"\\&");
      device = device.replace(/\|/g,"\\|");
      device = device.replace(/ /g,"\\ ");

      _deviceName =  escapeBase64InString(device);
		command += "query";
		// set64query
		// the global _isSendToAll is false for this encodeQuery
	   _isSendToAll = false;
	}
	else
	{
		// the global _isSendToAll is modified based on the idDevices state..
	   setupDevice(idDevices);
   }

	// encode the device as well.
	var commandPath = getCommandPath(command);

	//! 1.11.24 kinda kludge. if set64device replace with set64device2 (if not encodeQuery)
	//! The web should have a command like "set64_2" that way the "getCommandPath()" is unchanged
	//! since it just tacks on the "device" part
	if (!encodeQuery)
	{
      commandPath = commandPath.replace("set64device","set64device2");
	}

	var cmd = document.getElementById(anySetCommandId).value;
	// strip spaces on cmd..
   cmd = cmd.replace(/\s+/g, '');
	var arg = document.getElementById(anySetValId).value;
	// escape the spaces
   arg =  escapeBase64InString(arg);
   //arg = arg.replace(/\s+/g, '');

   //<!-- dawgpackTopicId used in the sendAnySetCommand64 --->
	var topicSet = document.getElementById('dawgpackTopicId').value;
   var isDawgpack = topicSet.localeCompare("dawgpack")==0;

	var path = commandPath + "/" + cmd + "/" + arg;
	if (isDawgpack)
	   path += "?topic=dawgpack";

   //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
   sendGETCommand(path);
}

//! wraps the command no args
function sendNoArgsM5(idDevices, idCmd)
{
	setupDevice(idDevices);

	// the command
	var cmd = document.getElementById(idCmd).value;
	var command = "cmd";
	var commandPath = getCommandPath(command);

	//! 1.13.24
	commandPath += getCurrentGroupNameQuery(false);

  //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
  sendGETCommand(commandPath + "/" + cmd);
}

//! cycles through pages
function cycleM5Pages(idDevices, idCmd)
{
	 // known last page..
    var currentPage = document.getElementById(idCmd).value;
	 if (currentPage.localeCompare(_lastSMPage)==0)
	    currentPage = "sm0";
	 else
	 {
	    // get the number part:
		 var num = parseInt(currentPage.substring(2));
		 num ++;
		 currentPage = "sm" + num;
	 }
    document.getElementById(idCmd).value = currentPage;
	 sendNoArgsM5(idDevices,idCmd);
}

//! cycles through DocFollow
function cycleDOCFOLLOW(idDevices, idSet, idCmd, idTextField)
{
    var selectIndex = document.getElementById(idCmd).selectedIndex;
	 selectIndex++;

	 //! update the next in the list..
	 var max = document.getElementById(idCmd).options.length;
	 selectIndex = selectIndex % max;


    document.getElementById(idCmd).selectedIndex = selectIndex;

	 //!update the text field for show
	 docFollowChanged(idCmd, idTextField);
    //document.getElementById(idTextField).value = document.getElementById(idCmd).value;


	 //! invoke the set command
    sendAnySetCommand64(idDevices, idSet, idCmd);
}

//! each time the option changes
function docFollowChanged(idCommandArg, idTextField) 
{
    var option = document.getElementById(idCommandArg).value;

	 //!update the text field for show
    document.getElementById(idTextField).value = option;
}

//! cycles through colors (idArg == text)
function cycleM5ScreenColors(idDevices, idCmd, idArg)
{
    var cmd = document.getElementById(idCmd).value;
	 if (cmd.localeCompare("screencolor")!= 0)
	    return;

	 // known last color..
	 var maxColor = 5;
    var currentColor = document.getElementById(idArg).value;
	 currentColor++;
	 currentColor = currentColor % maxColor;
    document.getElementById(idArg).value = currentColor;
	 sendTextArgs(idDevices,idCmd,idArg);
}

//! wraps the command arg == setDevice 
//! if isBooleanArg, then this is a set:cmd, val:boolValue
//!  Otherwise it's a cmd:cmd{ON/OFF}
//! All commands here are on/off version
function sendBooleanArgs(idDevices, idCmd, idArg)
{
	//
	setupDevice(idDevices);

	// the command, either set or cmd
	var cmd = document.getElementById(idCmd).value;
	var arg = document.getElementById(idArg).value;
	var commandPath = "";

	//! command  set:cmd, val:val
	//!    setDevice/n/u/d/CMD/VAL
   command = "set";
	commandPath = getCommandPath(command) + "/" + cmd + "/" + arg;

	//! 1.13.24
	commandPath += getCurrentGroupNameQuery(false);

  //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
  sendGETCommand(commandPath);
}


//! wraps the command arg == setDevice 
//!  set:cmd, val:arg
function sendTextArgs(idDevices, idCmd, idArg)
{
	setupDevice(idDevices);

	// the command
	var command = "set";
	var commandPath = getCommandPath(command);

	var cmd = document.getElementById(idCmd).value;
	var arg = document.getElementById(idArg).value;
	// strip spaces on arg..

	if ("location".localeCompare(cmd)==0)
      arg = escapeSpacesInString(arg);
	else
      arg = arg.replace(/\s+/g, '');


	// if an empty string then use special _EMPTY_ keyword 
	// to be converts by node-red back to ""
	if (arg.localeCompare("")==0)
	   arg = "_EMPTY_";

	//! 1.13.24
	arg += getCurrentGroupNameQuery(false);

   //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
  sendGETCommand(commandPath + "/" + cmd + "/" + arg);
}


//! wraps the command arg == setDevice 
//!  set:cmd, val:arg
function showSemanticMarker(idDevices)
{
	setupDevice(idDevices);

	// the command
	var command = "set";
	var commandPath = getCommandPath(command);

	var cmd = "SMZoom";
	var arg = "on";
	// strip spaces on arg..
   arg = arg.replace(/\s+/g, '');

	//! 1.13.24
	arg += getCurrentGroupNameQuery(false);

   //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
   sendGETCommand(commandPath + "/" + cmd + "/" + arg);
}

//! test update for OTATarget (M5, ESP-BOARD, ESP-ORIG)
function updateOTATest(idDevices, OTATarget)
{
   
   var deviceName = document.getElementById(idDevices).value;

   //if (confirm("Confirm: OTA of " + OTATarget + "\nFor Device: " + deviceName + "\nThis takes about 2 to 5 minutes") == true)
	//{
   	setupDevice(idDevices);
   
   	//rootOTAFile
   	var command = "rootOTAFile";
   	var commandPath = getCommandPath(command);
   
   	// now tack on the rest..  ROOT and the VAL
   	if (OTATarget.localeCompare("M5")==0)
   	{
         // M5
   		commandPath += "/M5/ESP_IOT.ino.m5stick_c_plus.bin";
   	}
   	else if (OTATarget.localeCompare("M5Camera")==0)
   	{
         // M5
   		commandPath += "/M5/ESP_IOT.ino.m5stack_timer_cam.bin";
   	}
      //new 9.27.23 for  M5 QR Reader
      else if (OTATarget.localeCompare("M5QRReader")==0)
      {
         // M5QRReader
         commandPath += "/M5QRReader/ESP_IOT.ino.m5stick_c_plus.bin";
      }
      //new 12.26.23 for  M5 Atom (power) Socket
      else if (OTATarget.localeCompare("M5AtomSocket")==0)
      {
         // M5AtomSocket
         commandPath += "/M5AtomSocket/ESP_IOT.ino.m5stick_c_plus.bin";
      }
      //new 1.4.24 for  M5 Atom -- all of them in 1
      else if (OTATarget.localeCompare("M5Atom")==0)
      {
         // M5Atom
         commandPath += "/M5Atom/ESP_IOT.ino.m5stick_c_plus.bin";
      }
      //new 1.25.24 for  M5 Core2 -- all of them in 1
      else if (OTATarget.localeCompare("M5Core2")==0)
      {
         // M5Core2
         commandPath += "/M5Core2/ESP_IOT.ino.m5stack_core2.bin";
      }
   	else if (OTATarget.localeCompare("ESP-ORIG")==0)
   	{
         // ORIG
   		commandPath += "/ORIG/ESP_IOT.ino.esp32.bin";
   	}
   	else if (OTATarget.localeCompare("ESP-BOARD")==0)
   	{
         // BOARD
   		commandPath += "/OTA_Board/ESP_IOT.ino.esp32.bin";
   	}


		//QA Versions
   	else if (OTATarget.localeCompare("QA_M5")==0)
   	{
         // M5
   		commandPath += "/QA_M5/ESP_IOT.ino.m5stick_c_plus.bin";
   	}
   	else if (OTATarget.localeCompare("QA_ESP-BOARD")==0)
   	{
         // BOARD
   		commandPath += "/QA_OTA_Board/ESP_IOT.ino.esp32.bin";
   	}

		//Main Versions
   	else if (OTATarget.localeCompare("MAIN_M5")==0)
   	{
         // M5
   		commandPath += "/MAIN/ESP_IOT.ino.m5stick_c_plus.bin";
   	}
   	else if (OTATarget.localeCompare("MAIN_ESP-BOARD")==0)
   	{
         // BOARD
   		commandPath += "/MAIN/ESP_IOT.ino.esp32.bin";
   	}

		else
		{
		   alert("Unknown OTA Target: " + OTATarget);
		}

 if (confirm("Confirm: OTA of " + OTATarget + "\n\nFor Device: " + deviceName + "\n\n" + commandPath + "\n\n\nThis takes about 2 to 5 minutes") == true)
   {
		//!NOTE: the node-red is looking for 2 argument (eg. MAIN / bin)
		//! Also, on KnowledgeShark.org/OTA there is a TEST and all these are based off TEST.. 
		//! NOTE: that isn't the usual spot for the default build.


		//! Result file will be at: 
		//!  http://KnowledgeShark.org/OTA/TEST/<command path>
   
      //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
      sendGETCommand(commandPath);

	}

}

//! Special case for renaming a device when the device name isn't in the list.. as it was already renamed
//! wraps the command arg == setDevice 
//!  set:cmd, val:arg
function sendRenameDevice(idDeviceToRename, idDeviceNewName)
{
   // strip the device name in case..
	var oldName = document.getElementById(idDeviceToRename).value;
   oldName = oldName.replace(/\s+/g, '');
	document.getElementById(idDeviceToRename).value = oldName;


	// now call the common setupDevice
	setupDevice(idDeviceToRename);

	// the command
	var command = "set";
	var commandPath = getCommandPath(command);

	var cmd = "device";
	var arg = document.getElementById(idDeviceNewName).value;
	// strip spaces on arg..
   arg = arg.replace(/\s+/g, '');

	// if an empty string then use special _EMPTY_ keyword 
	// to be converts by node-red back to ""
	if (arg.localeCompare("")==0)
	   arg = "_EMPTY_";

   //! sends command on HTTP
   sendGETCommand(commandPath + "/" + cmd + "/" + arg);
}

//! Special case sending any set:val commands
//!  set:cmd, val:arg
function sendAnySetCommand(idDevices, anySetCommandId, anySetValId)
{
	setupDevice(idDevices);

	// the command
	var command = "set";
	var commandPath = getCommandPath(command);

	var cmd = document.getElementById(anySetCommandId).value;
	// strip spaces on cmd..
   cmd = cmd.replace(/\s+/g, '');
	var arg = document.getElementById(anySetValId).value;
	// escape the spaces
   arg =  escapeSpacesInString(arg);
   //arg = arg.replace(/\s+/g, '');

	// if an empty string then use special _EMPTY_ keyword 
	// to be converts by node-red back to ""
	if (arg.localeCompare("")==0)
	   arg = "_EMPTY_";

	//! 1.13.24
	arg += getCurrentGroupNameQuery(false);

   //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
  sendGETCommand(commandPath + "/" + cmd + "/" + arg);
}

//! Special case sending any set:val commands
//!  set:cmd, val:arg
//! THese use the values passed in for 2nd and 3rd parm
function sendAnySetCommand2(idDevices, anySetCommand, anySetVal)
{
	setupDevice(idDevices);

	// the command
	var command = "set";
	var commandPath = getCommandPath(command);

	var cmd = anySetCommand;
	// strip spaces on cmd..
   cmd = cmd.replace(/\s+/g, '');
	var arg = anySetVal;

	// escape the spaces
   arg =  escapeSpacesInString(arg);
   //arg = arg.replace(/\s+/g, '');

	// if an empty string then use special _EMPTY_ keyword 
	// to be converts by node-red back to ""
	if (arg.localeCompare("")==0)
	   arg = "_EMPTY_";


   //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
  sendGETCommand(commandPath + "/" + cmd + "/" + arg);
}

//! Special case sending any set:val commands
//!  set:cmd, val:arg
//! This version converts the val into base64, and the command is added with 64
function sendAnySetCommand64(idDevices, anySetCommandId, anySetValId)
{
	//TODO refactor..
   sendAnySetCommand64Both(idDevices, anySetCommandId, anySetValId, false);
	return;
}

//! wraps the command arg == setDevice 
//!  set:cmd, val:"idSSID:idPassword"
function sendAddWIFITextArgs(idDevices, idSSID, idPassword)
{
	setupDevice(idDevices);

	// the command
	var command = "set";
	var commandPath = getCommandPath(command);

	var cmd = "addwifi";
	var ssid = document.getElementById(idSSID).value;
	var password = document.getElementById(idPassword).value;
	var arg = ssid + ":" + password;
	// strip spaces on arg..
   arg = arg.replace(/\s+/g, '');

	// if an empty string then use special _EMPTY_ keyword 
	// to be converts by node-red back to ""
	if (arg.localeCompare("")==0)
	   arg = "_EMPTY_";

   //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
  sendGETCommand(commandPath + "/" + cmd + "/" + arg);
}

//! grab the QRAvatar first..
//! http://www.javascriptkit.com/javatutors/javascriptpromises.shtml
function getImage(url)
{
    console.log("getImage: " + url);
    return new Promise(function(resolve, reject)
	 {
        var img = new Image()
        img.onload = function()
		  {
            console.log("image.onload");
            resolve(url)
        }
        img.onerror = function()
		  {
            reject(url)
        }
        img.src = url
    })
}

//!create a semantic marker, and place at the idSM location
function createSemanticMarker(idDevices,idSemanticMarkerImageURL,idSM, idLabel)
{
	setupDevice(idDevices);

   // this will use guest password..
	var command = "feedguest";
	var commandPath = getCommandPathGuest(command);
	// feedguest<device>/USER/GPASS/<device>
   document.getElementById('_messageId').innerHTML = commandPath;

   var circular = document.getElementById('boolArgValueCircular').value=="on";

	var avatarURL = document.getElementById(idSemanticMarkerImageURL).value;

	//no avatarURL so use iDogWatch
   if (!avatarURL || avatarURL.length==0)
	{
	  //hard wire for now..
	  avatarURL = "https://idogwatch.com/pettutor/iDogWatch.png";
	}

	//empty the id first..
	//Seems that the call is adding a new SM instead of cleaning it up..
	document.getElementById(idSM).value = "";
	document.getElementById(idSM).html = "<p>x</p>";


   document.getElementById(idSM).innerHTML = "";

   console.log(idSM);
   console.log(document.getElementById(idSM));

	//! fix bug: https://itecnote.com/tecnote/javascript-qrcode-js-error-code-length-overflow-17161056/
	if (commandPath.length >= 192 && commandPath.length <= 220)
	{
      commandPath = commandPath + "&filler=";

      // pad to greater thatn 220
		var len = commandPath.length;
		while (commandPath.length <= 220)
		{
		   commandPath= commandPath + "x";
		}
	}

	try
	{
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
 
		      alert("Issue creating QRAvatar in Semantic Marker creation: \n" + errorurl + "\n\nPlease file bug report"
				   + "\nsyntax including  # can be problematic");
			})
	}
	catch (error)
	{

		alert("Issue with Semantic Marker creation: \n" + error.message + "\nPlease file bug report");
	}

   document.getElementById(idLabel).innerHTML = commandPath;
}


//!create a semantic marker, and place at the idSM location
function createSemanticMarkerPath(commandPath, avatarURL, idSM, idLabel)
{
   var circular = document.getElementById('boolArgValueCircular').value=="on";

	//avatar URL
	//var url = document.getElementById(idSemanticMarkerImageURL).value;

	//no avatarURL so use iDogWatch
   if (!avatarURL || avatarURL.length==0)
	{
	  //hard wire for now..
	  avatarURL = "https://idogwatch.com/pettutor/iDogWatch.png";
	}

	//empty the id first..
	var previousSM = document.getElementById(idSM);
	//Seems that the call is adding a new SM instead of cleaning it up..
	document.getElementById(idSM).value = "";
	//document.getElementById(idSM).html = "<p>x</p>";
	document.getElementById(idSM).html = "<p >x</p>";

   document.getElementById(idSM).innerHTML = "";

   console.log(idSM);
   console.log(document.getElementById(idSM));

	//! fix bug: https://itecnote.com/tecnote/javascript-qrcode-js-error-code-length-overflow-17161056/
	if (commandPath.length >= 192 && commandPath.length <= 220)
	{
      commandPath = commandPath + "&filler=";

      // pad to greater thatn 220
		var len = commandPath.length;
		while (commandPath.length <= 220)
		{
		   commandPath= commandPath + "x";
		}
	}

	try
	{
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
 
		      alert("Issue creating QRAvatar in Semantic Marker creation: \n" + errorurl + "\n\nPlease file bug report"
				  + "\nUrls with # and other special characters can be problematic");
			})

	}
	catch (error)
	{

		alert("Issue with Semantic Marker creation: \n" + error.message + "\nPlease file bug report");
	}

   //document.getElementById(idLabel).value = commandPath;
   document.getElementById(idLabel).innerHTML = commandPath;

}

//!create a semantic marker, and place at the idSM location
function createSemanticMarkerPathId(commandPathId, urlId ,idSM, idLabel)
{
   var commandPath = document.getElementById(commandPathId).value;
	var url = document.getElementById(urlId).value;
   if (!url || url.length==0)
	{
	   url = "https://SemanticMarker.org/images/SM-Circle-R.png"; 
	}
   createSemanticMarkerPath(commandPath, url ,idSM, idLabel);
}

//! 12.29.23 add Credentials
//!create a semantic marker, and place at the idSM location
//! 1.1.24 added device name
function createCredentialsSemanticMarkerPathId(smURLPathId, urlId ,idSM, idLabel, idSSID, idSSIDPass, idDeviceName)
{
	var ssidString = document.getElementById(idSSID).value;
	    ssidString = encodeURIComponent(ssidString);
	var ssidStringPassword = document.getElementById(idSSIDPass).value;
	    ssidStringPassword = encodeURIComponent(ssidStringPassword);
	var deviceNameString = document.getElementById(idDeviceName).value;
	   deviceNameString = encodeURIComponent(deviceNameString);

	var createFull = false;
	var addDeviceName = false;
	if (window.confirm("Cancel: Create just WIFI Credentials\n\n" 
	    + "   ssid        ='" + ssidString + "'\n"
	    + "   ssidPassword='" + ssidStringPassword + "'\n\n"
	    + "OK: create full credentials exposing your password\n\n"
		 ))
	{
	    createFull = true;
	    if (window.confirm("Cancel: do not override device name\n\n"
    		 + "OK: to define\n"
	        + "   deviceName  ='" + deviceNameString + "'" + "\n\n"
			  )
		 )
		 {
		    addDeviceName = true;
		 }

	}

	//! semantic marker address for credentials
   var commandPath = "https://SemanticMarker.org/bot/credentials";

	commandPath += "?ssid=" + ssidString;
	commandPath += "&ssidpass=" + ssidStringPassword;

	if (createFull)
	{
	   commandPath += "&username=" + _username;
	   commandPath += "&password=" + _password;
		if (addDeviceName)
	      commandPath += "&deviceName=" + deviceNameString;

		//todo .. add location
	}

//! TODO: see if not defining the deviceName works..

	document.getElementById(smURLPathId).value = commandPath;

	var url = document.getElementById(urlId).value;
   if (!url || url.length==0)
	{
	   url = "https://SemanticMarker.org/images/SM-Circle-R.png"; 
	}
   createSemanticMarkerPath(commandPath, url ,idSM, idLabel);
}

//! 12.31.23
//! 1.2.24 add SemanticMarker as option
// create: /bot/scannedDevice/DeviceName?kind=AtomSocket&status=socket&valKind=onoff
//!create a semantic marker, and place at the idSM location
function setupM5AtomKind(idM5AtomKindOption, idKind, idStatus, idValKind)
{
	var atomKind = document.getElementById(idM5AtomKindOption).value;
	var kindString;
	var statusString;
	var valKindString;
	if (atomKind == 'm5socket')
	{
	   kindString = "AtomSocket";
		statusString = "socket";
		valKindString = "onoff";
	}
	else if (atomKind == "m5scanner")
	{
	   kindString = "AtomScanner";
		statusString = "smscanner";
		valKindString = "boolean";
	}
	else if (atomKind == "SemanticMarker")
	{
	   if (!_lastCopiedSM)
		{
         alert("No Semantic Marker&trade; selected. Select one and click 'Copy SM' Above");
			return;
		}
		kindString = "SemanticMarker";
		statusString = _lastCopiedSM.uuid;
		valKindString = _lastCopiedSM.flowNum;
	}

	document.getElementById(idKind).value = kindString;
	document.getElementById(idStatus).value = statusString;
	document.getElementById(idValKind).value = valKindString;
}

//! 12.31.23
//! 1.2.24 add the last copied SM to the list (which will fill the status and valKind fields)
// create: /bot/scannedDevice/DeviceName?kind=AtomSocket&status=socket&valKind=onoff
//!create a semantic marker, and place at the idSM location
function createScannedDeviceSemanticMarkerPathId(smURLPathId, urlId ,idSM, idLabel, idKind, idStatus, idValKind, idDevices)
{
	// set the device =>  _deviceName
   setupDevice(idDevices)
   var commandPath = "https://SemanticMarker.org/bot/scannedDevice/" + _deviceName;

	var kindString = document.getElementById(idKind).value;
	var statusString = document.getElementById(idStatus).value;
	var valKindString = document.getElementById(idValKind).value;
	commandPath += "?kind=" + kindString;

	if (kindString == "SemanticMarker")
	{
	   commandPath += "&uuid=" + statusString;
	   commandPath += "&flow=" + valKindString;
	}
	else
	{
	   commandPath += "&status=" + statusString;
	   commandPath += "&valkind=" + valKindString;
	}
	document.getElementById(smURLPathId).value = commandPath;

	var url = document.getElementById(urlId).value;
   if (!url || url.length==0)
	{
	   url = "https://SemanticMarker.org/images/SM-Circle-R.png"; 
	}
   createSemanticMarkerPath(commandPath, url ,idSM, idLabel);
}

//! 1.7.24
// create: /bot/scannedGroup/GroupName
//!create a semantic marker, and place at the idSM location
function createScannedGroupSemanticMarkerPathId(smURLPathId, urlId ,idSM, idLabel, idGroups)
{
	var groupName = document.getElementById(idGroups).value;
	var validGroup = true;
	if (!groupName || groupName.length == 0)
	{
      validGroup = false;
	}
	else if (groupName == "NO_GROUP")
	   validGroup = false;

	if (!validGroup)
	{
	   alert("Please select a valid group name");
		return;
	}
   var commandPath = "https://SemanticMarker.org/bot/scannedGroup/" + groupName;

	document.getElementById(smURLPathId).value = commandPath;

	var url = document.getElementById(urlId).value;
   if (!url || url.length==0)
	{
	   url = "https://SemanticMarker.org/images/SM-Circle-R.png"; 
	}
   createSemanticMarkerPath(commandPath, url ,idSM, idLabel);
}

//! 12.30.23 add Credentials but in WIFI format
//!          WIFI:S:<SSID>;T:<WEP|WPA|blank>;P:<PASSWORD>;H:<true|false|blank>;
//!create a semantic marker, and place at the idSM location
function createWIFICredentialsSemanticMarkerPathId(smURLPathId, urlId ,idSM, idLabel, idSSID, idSSIDPass)
{
   var commandPath = "WIFI:";

	// base64 encode .. seems standards doesn't encode them
	var encodeArgs = false;

	var ssidString = document.getElementById(idSSID).value;
	var ssidStringPassword = document.getElementById(idSSIDPass).value;
	if (encodeArgs)
	{
	    ssidString = encodeURIComponent(ssidString);
	    ssidStringPassword = encodeURIComponent(ssidStringPassword);
	}

	commandPath += "S:" + ssidString + ";";
	commandPath += "T:WPA" + ";";
	commandPath += "P:" + ssidStringPassword + ";";
	commandPath += "H:" + ";";


	document.getElementById(smURLPathId).value = commandPath;

	var url = document.getElementById(urlId).value;
   if (!url || url.length==0)
	{
	   url = "https://SemanticMarker.org/images/SM-Circle-R.png"; 
	}
   createSemanticMarkerPath(commandPath, url ,idSM, idLabel);
}

//! BOT DEVICE MANAGEMENT (add, remove, rename)
//!adds the device (from the deviceName .. to the user)
function addDeviceNameBot(deviceNameId)
{

	//! this is so the 'device' isn't tacked on..
	_isSendToAll = true;

   // strip spaces
   var deviceName = document.getElementById(deviceNameId).value;
   deviceName = createValidDeviceName(deviceName);

   var command = "adddevice";
   var commandPath = getCommandPath(command);

   //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
   sendGetCommand_thenAlert(commandPath + "/" + deviceName, "Added device: " + deviceName, true);
}

//!removes the device (from the deviceName .. to the user)
function removeDeviceNameBot(deviceNameId)
{
	//! this is so the 'device' isn't tacked on..
	_isSendToAll = true;

   // strip spaces
   var deviceName = document.getElementById(deviceNameId).value;
   deviceName = createValidDeviceName(deviceName);

   var command = "removedevice";
   var commandPath = getCommandPath(command);

   //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
   sendGetCommand_thenAlert(commandPath + "/" + deviceName, "Removed device: " + deviceName, true);

}


//!renames the device (from the deviceName .. to the user)
function renameDeviceNameBot(deviceNameId, deviceNameNewId)
{
	//! this is so the 'device' isn't tacked on..
	_isSendToAll = true;

   var deviceNameNew = document.getElementById(deviceNameNewId).value;
   deviceNameNew = createValidDeviceName(deviceNameNew);

   // strip spaces
   var deviceName = document.getElementById(deviceNameId).value;
   deviceName = createValidDeviceName(deviceName);

   var command = "renamedevice";
   var commandPath = getCommandPath(command);

   //! https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/send
   sendGetCommand_thenAlert(commandPath + "/" + deviceName + "/" + deviceNameNew, "Renamed device: " + deviceName + " to:" + deviceNameNew, true);
}
//!END DEVICE MANAGEMENT

//! sends the GET command, but shows an alert when done..
function sendGetCommand_thenAlert(command, alertString, doRefresh)
{
   var xhttp = new XMLHttpRequest();

	//!https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readystatechange_event
   xhttp.onreadystatechange = function() {
		// called when open is finished
      if (this.readyState == 4 && this.status == 200) 
	   {
		   if (alertString && alertString.length > 0)
			{
		     alert(alertString);
           if (doRefresh)
			  {
			     //location.reload();
				  // 6.13.23 use the reload of agent data, not the web page
				  // Especially if the stateless version is used
              reloadUserAgent();
			  }
			}
      }
   };
  //! update the messageId too
  document.getElementById('_messageId').innerHTML = command;

//TRYING value (not innerHTML)
  // Update the text window..
  var recordedTextMessages = document.getElementById('_recordedMessages').value;
  recordedTextMessages += command + "\n";
  document.getElementById('_recordedMessages').value = recordedTextMessages;


  xhttp.open("GET", command, true);
  xhttp.send();
}

//! sends the GET command, but shows an alert when done..
function justSendGETCommand(command)
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


//! set when to user the parameters
//! FOR NOW: since the command was already created,
//!   The command will be searched for the the values [0],[1] and replace with the parameters [0],[1]
var _useSemanticMarkerParameters = true;

//! Gets the command
//!  HERE is a device is mentioned .. add DEVICE for the current device.. TODO
function getParameterizedSemanticMarkerCommand(val, useSemanticMarkerParameters)
{
	//! the arg is the val passed in.. which is the complete command iDogwatch.com/bot/...
	var arg = val;

   //!   The command will be searched for the the values [0],[1] and replace with the parameters [0],[1]
	//! The issue: this arg is in the context of some random URL (not a SemanticMarker URL). So the parameters are in that context
	//! BUT: how do we get the SemanticMarker engine to manipulate and instantiate it?
	//!   It seems this URL must be wrapped AGAIN in base64 .. and that sent as a SemanticMarker to the SemanticMarker engine
	//!NOTE: the arguments USERNAME and PASSWORD are known to be the current users _username and _password

   if (useSemanticMarkerParameters)
	{
	   //! update the UUID
		updateSemanticMarkerUUID();

	   for (var i=0; i< _SemanticMarkerValues.length; i++)
		{
		   arg = arg.replace( _SemanticMarkerValues[i],_SemanticMarkerParameters[i]);
		}
      console.log("SemanticAddress cmd = " + arg);

		//!result is:
		//!   https://SemanticMarker.org/bot/sm/uuid/<UUID>/ ... rest of command ...
		//! NOTE: This assumes the final command goes through the SemanticMarker (versus some random location).

	   // escape the spaces. Convert to Base64
      arg = escapeBase64InString(arg);

		// Now create a new ARG , which contains the previous command (eg. dogwatch/feed already 
		// Modified to for the parameters (Username, password ..etc)
		// ??? Why is UUID needed? It seems the ARG has been intantiated.. the UUIR is used to get the parameters (not here yet)

		arg = "https://SemanticMarker.org/bot/sm/" + _SemanticMarker_UUID + "/" + arg;

	}

   return arg;

}

//! repeats the GET command as a semanticMarker Docfollow message (send to ALL)
//! pass the VAl of the URL just sent
function sendSemanticMarkerCommand(val)
{
	//! 9.28.23 going to add the "dev" to the send .. NO wildcard yet
	// send to all
//	_isSendToAll = true;

	//!setup the devices
   setupDevice('devices');

	// the command (base) .. device is tacked on 
	var command = "set64";

	// encode the device as well.
	var commandPath = getCommandPath(command);

	console.log("commandPath = " + commandPath);

   //var arg = getParameterizedSemanticMarkerCommand(val, _useSemanticMarkerParameters);
	//! the arg is the val passed in.. which is the complete command iDogwatch.com/bot/...
	var arg = val;

   //!   The command will be searched for the the values [0],[1] and replace with the parameters [0],[1]
	//! The issue: this arg is in the context of some random URL (not a SemanticMarker URL). So the parameters are in that context
	//! BUT: how do we get the SemanticMarker engine to manipulate and instantiate it?
	//!   It seems this URL must be wrapped AGAIN in base64 .. and that sent as a SemanticMarker to the SemanticMarker engine
	//!NOTE: the arguments USERNAME and PASSWORD are known to be the current users _username and _password

   if (_useSemanticMarkerParameters)
	{
	   //! update the UUID
		updateSemanticMarkerUUID();

	   for (var i=0; i< _SemanticMarkerValues.length; i++)
		{
		   arg = arg.replaceAll( _SemanticMarkerValues[i],_SemanticMarkerParameters[i]);
		}
      console.log("SemanticAddress cmd = " + arg);

		//!result is:
		//!   https://SemanticMarker.org/bot/sm/uuid/<UUID>/ ... rest of command ...
		//! NOTE: This assumes the final command goes through the SemanticMarker (versus some random location).

	   // escape the spaces. Convert to Base64
      arg = escapeBase64InString(arg);

		// Now create a new ARG , which contains the previous command (eg. dogwatch/feed already 
		// Modified for the parameters (Username, password ..etc)
		// ??? Why is UUID needed? It seems the ARG has been intantiated.. the UUID is used to get the parameters (not here yet)

		arg = "https://SemanticMarker.org/bot/sm/" + _SemanticMarker_UUID + "/" + arg;

	}

	// escape the spaces. Convert to Base64
   arg = escapeBase64InString(arg);

   //arg = arg.replace(/\s+/g, '');

   //<!-- dawgpackTopicId used in the sendAnySetCommand64 --->
	var topicSet = document.getElementById('dawgpackTopicId').value;
   var isDawgpack = topicSet.localeCompare("dawgpack")==0;

	var cmd = "SemanticMarker";

	var path = commandPath + "/" + cmd + "/" + arg;
	if (isDawgpack)
	   path += "?topic=dawgpack";

	console.log("path = " + path);


   //sendGETCommand(path);  (Cannot.. recursion..)
   sendGetCommand_thenAlert(path,"", false);
}


//! sends the GET command
function sendGETCommand(command)
{

	var arg1 = document.getElementById('_boolArgValueOnlySM').value;
	if ("off".localeCompare(arg1)==0)
	{
	   // call the send without an alert string
      sendGetCommand_thenAlert(command,"", false);
	}

	//! grab the boolean switch..
	var arg = document.getElementById('_boolArgValueRepeatSM').value;
	if ("on".localeCompare(arg)==0 || "on".localeCompare(arg1)==0)
	{
	   // just send SemanticMarker 
	   sendSemanticMarkerCommand(command);
	}
}

//! invokes a web site specified, bringing up a web window
function invokeWebSite(idWebAddress)
{
   var webAddress = document.getElementById(idWebAddress).value;

   //! display in new window
	//window.open(webAddress,"WEB");
	window.open(webAddress);
}

//! { elementId, initialString }
var _registeredDeviceChanges = [];

//! add the objects to change when the device name changes..
//! { elementId, initialString }
function registerDeviceChange(elementId, initialString)
{
    var object = {'elementId':elementId, 'initialString':initialString};
	 _registeredDeviceChanges.push(object);

}

//! change devices big button
function devicesChanged(idDevices)
{
   var deviceName = document.getElementById(idDevices).value;

   //9.5.23 change the color if special ones..
   if (deviceName.startsWith("M5"))
      document.getElementById(idDevices).className= "M5button";
   else if (deviceName.includes("GEN3"))
      document.getElementById(idDevices).className= "H2orange";
   else
      document.getElementById(idDevices).className= "btnB";

   for (var i =0; i< _registeredDeviceChanges.length; i++)
	{
	   var elementId = _registeredDeviceChanges[i].elementId;
	   var initialString = _registeredDeviceChanges[i].initialString;

	   var label = initialString + ": " + deviceName;
      document.getElementById(elementId).innerHTML = label;

	}
}

//!init registered devices
function initRegisteredDevices()
{
   registerDeviceChange('_manageActualDevice',"Manage Actual Device"); 
   registerDeviceChange('_M5SpecificCommands',"M5 Specific Commands");
   registerDeviceChange("_TimerCommands","Timer Commands to your device");
   registerDeviceChange("_CommandsToYourDevice", "Commands to your Device");
   registerDeviceChange("_DeviceNamesManagedByBot", "Device Names & Guest Passwords managed by your Agent (bot)");

   registerDeviceChange("_DOCFOLLOWMessages", "DOCFOLLOW SemanticMarker&#x2122; Example Messages");
   registerDeviceChange("_RevertToMainOTA", "Revert to MAIN OTA");
   registerDeviceChange("_QAOTA", "QA OTA Test Updates");
   registerDeviceChange("_DevOTA", "DEV OTA Test Updates");
   registerDeviceChange("_SMRecordDevice", "Device");


   registerDeviceChange("_GroupCommandsTab", "GROUP Messaging");

	//! 3.3.24 OTA
   registerDeviceChange ("_OTATab", "OTA Over the Air Updates");

	//! 3.3.24 add the SM Matrix
   registerDeviceChange ("_SMMatrixTab", "Semantic Marker&trade; Matrix");
}


//! just web invokes the SemanticMarker
function invokeSemanticMarker(smId)
{
	var webAddress = document.getElementById(smId).value;

	if (window.confirm("Invoke: " + webAddress + "?"))
	{
	   // invoke this manually..
      window.open(webAddress, 'SemanticMarkerWindow');
	}
}

//!invokes flow using info..
function invokeSavedFlow(recordedMessagesId, uuidStringTextId, flowNumberTextId, privateTextId)
{
   if (!_userFlowsJSON)
	{
      alert("Please click 'Get User SM Flows' to initialize your SMART Buttons");
	   return;
	}
   // This has to sent this message backend..
	//  eg.  runFlow
   var flowNum = document.getElementById(flowNumberTextId).value;
   var uuid = document.getElementById(uuidStringTextId).value;
	//! format != json .. so this is a web call (to the semanticMarkerInstance.html)
	var webAddress = _semanticMarkerFlowURL + "smart?uuid=" + uuid + "&flow=" + flowNum;
	console.log("web = " + webAddress);

	// invoke this manually..
   window.open(webAddress, 'SemanticMarkerWindow');
}

//! calls the node-red process engine to create a smart deck .. then refreshes the decks
//!-- calls: /adddeckuuid/:NAME/:PASSWORD/:DECKNAME/:DECKPASSWORD/:DECKCAT -->
function createSmartDeck(smartDeckCategoryId,smartDeckNameId,smartDeckPasswordId)
{
   var deckcat = document.getElementById(smartDeckCategoryId).value;
   deckcat = createValidDeviceName(deckcat)

   var deckname = document.getElementById(smartDeckNameId).value;
   deckname = createValidDeviceName(deckname)

   var deckpass = document.getElementById(smartDeckPasswordId).value;
   deckpass = createValidDeviceName(deckpass)

	if (deckcat.length == 0 || deckpass.length == 0 || deckpass == 0)
	{
      alert("Values must be not be empty (and spaces are removed)");
		return;
	}

	var msg = "Create SMART Deck? \n" + "Name: " + deckname + "\nCat: " + deckcat + "\nPass: " + deckpass;
	if (window.confirm(msg))
	{
       var webAddress;
if (_useOldSMARTDeck)
{
       //! send this as a GET command to the bot backend (nodered)
       webAddress = _semanticMarkerFlowURL + "adddeckuuid/" + _username + "/" + _password  
		   + "/" + deckname + "/" + deckpass + "/" + deckcat;
}
else
{
       //! send this as a GET command to the bot backend (nodered)
       webAddress = _semanticMarkerFlowURL + "adddeckuuidNEW/" + _username + "/" + _password  
		   + "/" + deckname + "/" + deckpass + "/" + deckcat;
}
   
       console.log("web = " + webAddress);
		 alert(webAddress);
   
   	 // send this to node-red  "smflow/saveflow"
       justSendGETCommand(webAddress);

		 //TODO: needs a refresh
		 //NOTE: this doesn't return the UUID created ..  needs to update the SMUsersDB
		 // {username, password smartDecks:[] << place here

       //getSMUserUUID(uuidStringTextId)

	}
	else
	{

      alert("Cancelled creating a SMART Deck");
	}
}



//!sends flow using info..
/* 
  This will grab all the commands from the text recorded messages and base64 them .. and send as 1 message 
   https://SemanticMarker.org/bot/sm/saveflow/FLOW64?uuid=UUID&flow=10    where flow == flow number
	// Maybe put the parameters, flowname, flow# into another Base64 JSON..
	ADD: username,password somewhere (either &username=XXX&password=YY
	Then to run:
	https://SemanticMarker.org/bot/smart?uuid=UUID&flow=10
  "Save Semantic Marker&#x2122; Flow*"
*/
function sendSavedFlow(recordedMessagesId, uuidStringTextId,flowNumberTextId, flowNameTextId, flowCatTextId, privateId, flowsSelectId, isDataId, descTextId, dateTextId, QRAvatarURLTextId, parameterValuesSMId, nextFlowURLId, 
KSMatrixId, dataSMId, artifactsSMId, markupId, languagesSMId, videoSMId, isCircularAvatarId,
audioSMId, locationSMId, inheritedSMId, deckSMId, isMessagingId, isIndirectSMId, indirectSMId,
KSWaveId, bridgeSMId, futureId)
{

   // grab the text.. then wrap the entire things
	var isDataFlag = document.getElementById(isDataId).value;
	var privateflag = document.getElementById(privateId).value;
   var flowNum = document.getElementById(flowNumberTextId).value;
   var flowName = document.getElementById(flowNameTextId).value;
   var flowCat = document.getElementById(flowCatTextId).value;

	var nextFlowURL = document.getElementById(nextFlowURLId).value;

	// 3.22.23 added
	var KSMatrixText = document.getElementById(KSMatrixId).value;
	var dataSM = document.getElementById(dataSMId).value;
	var artifactsSM = document.getElementById(artifactsSMId).value;

	var KSWaveText = document.getElementById(KSWaveId).value;
	var bridgeSM = document.getElementById(bridgeSMId).value;
	var futureText = document.getElementById(futureId).value;
	var languagesSM = document.getElementById(languagesSMId).value;
	var videoSM = document.getElementById(videoSMId).value;
	var isCircularAvatar = document.getElementById(isCircularAvatarId).value;

	//! 1.9.24 .. warn the user if their 0 and 1 values are the same (username, password)
	if (_SemanticMarkerValues.length > 1)
	{
		// this assumes [0] and [1] are username/paswword in some order..
	   if (_SemanticMarkerValues[0] == _SemanticMarkerValues[1])
		{
		   if (!confirm("**** SMART Limitation as of 1.9.24: \n\nYour Username and Password are the same\nSo substituting for parameters is not supported yet. \n\nOK: SAVE anyway\n\nCancel: Don't save"))
			{
				alert("*** CANCELING saving this SMART button. Maybe change your password");
			   return;
			}
		}
	}

	// grab the UUID
   var uuid = document.getElementById(uuidStringTextId).value;
	//require a uuid
	if (!uuid || uuid.length == 0)
	{
      alert("UUID required");
		return;
	} 
	else 
	{
		var validUUID = false;
	   for (let i =0; i< _UUIDNumbers.length; i++)
		{
		   if (_UUIDNumbers[i].localeCompare(uuid)==0)
			{
			   validUUID = true;
				break;
			}
		}
		if (!validUUID)
		{
         alert(uuid + ": Cannot save a different users SMART Deck." 
		   + " Please inherit and clone to"
		   + " run your own (which is recommended).");
		   return;
		}
	}

	// NOTE: multiple lines don't work .. 
   var markupText = document.getElementById(markupId).value;
	// Replace NEWLINE with \n\
   markupText = markupText.replaceAll('\\n','\\n\\');

	var audioSM = document.getElementById(audioSMId).value;
	var locationSM = document.getElementById(locationSMId).value;
	var inheritedSM = document.getElementById(inheritedSMId).value;
	//var isDeck = document.getElementById(isDeckId).value;
	var deckSM = document.getElementById(deckSMId).value;
	var isMessaging = document.getElementById(isMessagingId).value;
	var isIndirectSM = document.getElementById(isIndirectSMId).value;
	var indirectSM = document.getElementById(indirectSMId).value;
	//! end added

	//5.1.23 .. check the deckSM syntax UUID.flownum, UUUID.flownum
	if (deckSM && deckSM.length > 0)
	{
		deckSM = stripSpaces(deckSM);
		let validDeckSM = true;
	   let deckSMparts = deckSM.split(",");
		for (let p=0;p<deckSMparts.length;p++)
		{
		   let deckSMpart = deckSMparts[p];
			let SMparts = deckSMpart.split(".");
			if (SMparts.length != 2)
			{
			  validDeckSM = false;
			  break;
			}
		}
		if (!validDeckSM)
		{
			alert("Invalid syntax for deckSM: " + deckSM 
			  + "\nComma seperated UUID.flownum required."
			  + "\n(Note: not checking for valid UUID.flownum)");
         return;
		}

	}
	//! 5.5.23
	//!   Seems that when saved, the .../DEVICE_1/ ... isn't detected as a parameter to look for.
	//!  unless the parameterValuesSMId .. is set
	//! 1.9.24  if username == password (eg. test) then the command substitution doesn't work. It could
	//! look a line at a time, and start with password (reverse) .. TODO
	//! BUT: going to remove the M5 and GEN3

	// select options 
	var flowParametersArray = ["USERNAME","PASSWORD"];
	var flowParameters = document.getElementById(parameterValuesSMId);
	 //! foreach arg.. replace the Parameter with the equilivent instance (eg. password -> _password)
	 for (var i=0; i< flowParameters.length; i++)
	 {
        var parameter = flowParameters[i].innerHTML;
		  flowParametersArray.push(parameter);
	 }
	 // now make them unique (new Set) and back to an array
	 var parameters  = Array.from(new Set(flowParametersArray));

	 //NOTE: 1.26.23   The parameters need to be examined below to see if they are still in the flow

	// NOTE: multiple lines don't work .. 
   var desc = document.getElementById(descTextId).value;
	// Replace NEWLINE with \n\
   desc = desc.replaceAll('\\n','\\n\\');

   var QRAvatarURL = document.getElementById(QRAvatarURLTextId).value;

	var currentDate = document.getElementById(dateTextId).value;


	//require a flowNum
	if (!flowNum || flowNum.length == 0)
	{
      alert("flowNumber required");
		return;
	} 

	//require a flowName
	if (!flowName || flowName.length == 0)
	{
      alert("flowName required");
		return;
	} 
	// strip spaces on flowName
   flowName = createValidFlowName(flowName);
   document.getElementById(flowNameTextId).value = flowName;

	//require a flowCat
	if (!flowCat || flowCat.length == 0)
	{
      alert("flow category required");
		return;
	} 
	// strip spaces on flowCat
   flowCat = createValidFlowName(flowCat);
   document.getElementById(flowCatTextId).value = flowCat;

	//encourage a QRAvatarURL
	if (!QRAvatarURL || QRAvatarURL.length == 0)
	{
		if (!window.confirm("The QRAvatarURL to an image is encouraged\nto improve visibility of your Semantic Marker\u00AE"))
		   return;
	} 

	//TODO: if a different flowNum .. then we are overriding .. so ask 

	// Check if this flow NUM is already defined .. and ask for confirmation to override..
	// Also check if the Name is used in the same Category.
	// SO Algorithm:  KEY = num (so cannot overlap without overwriting) 
	//    1. check if same num, set matchingNum=true
	//       so: will only overwrite if same cat and name,
	//       otherwise: overwrite not allowed
	//         1b. check if same cat, 
	//         1c. check if same name
	//       if all 3 -- then ask for Over Write 
	//         1d. if same CAT .. then RENAMING "name"
	//       -- otherwise cannot overwrite
	//    2. if no matching num 
	//     2.a  ensure flowName not in the same Category
	//  5.a - if same name and cat, but different flowNum .. then ask to change the flowNum
	//  6.a - if changing the category .. let them..
	//       
	if (_userFlowsJSON)
	{
	  // if override, then we are good.. otherwise
	  //  2.a ensure flowName not in same category
	  var override = false;
     for (var i =0; i< _userFlowsJSON.length; i++)
	  {
		  var flow = _userFlowsJSON[i];
		  if (flow.flowNum == flowNum)
		  {
		     //1.b. check same cat, 1.c same name
   		  if ((flow.name.localeCompare(flowName) == 0) && (flow.flowCat.localeCompare(flowCat) == 0))
			  {
			     if (window.confirm("** CAUTION **\nUpdating existing flow\nFlowName: " + flowName + "\nFlowCat: " + flow.flowCat + "\nFlowNum: " + flowNum))
			     {
		           console.log("**** Overriding a existing flow( " + flowNum + ") = " + flowName);

		           // if valid, then break the loop. There can be only 1 matching flowNum as it's our KEY
					  override = true;
				     break;
			     }
			     else
			     {
		           console.log("**** Canceled Overriding a existing flow: " + flowNum);
				     return;
			      }
				}
				// 1.d. Renaming name
   		   else if (flow.flowCat.localeCompare(flowCat) == 0)
				{
			     if (window.confirm("** CAUTION **\nRenaming flow\nOld   Name: " + flow.name 
				            + "\nNew Name: " + flowName + "\nFlowCat: " + flow.flowCat))
				  {
		           // if valid renaming, then break the loop. There can be only 1 matching flowNum as it's our KEY
					  override = true;
					  break;
				  }
				  else
				  {
				     alert("** Not Renaming **\nFlowNum (" + flowNum + ") already exists as\nFlowName: " + flow.name + "\nFlowCat: " + flow.flowCat);
					  return;
				  }

				}

				else
				{
				   alert("** Not Saving **\nFlowNum (" + flowNum + ") already exists as\nFlowName: " + flow.name + "\nFlowCat: " + flow.flowCat);
					return;
				}
		  } // if same num
	  }
	  
   // the node-red side checks for updating a flowNum, and if not there .. then adds. But
	// we are changing the number .. so there will be duplicates.
	//TODO:   delete in node-red first .. then add (but as that's an async web call tricky)
	// OR: the post message has a field "oldFlowNum" .. that is used.
/*
 //backend code:
 //!remove all flowNum == flowNum (vs replace)
   for (let [i, flow] of flows.entries()) {
      if (flow.flowNum === flowNum) {
         console.log("Remove: " + i);
         flows.splice(i, 1); // flowNum is now removed from "flows"
      }
   }
   //!add new flow
   msg.payload.flows.push(msg.flow);
*/
	  //! 5.3.23 .. check for changing a flow number
	  if (!override)
	  {
	  //bug..  not updating .. but creating another..
        for (var i =0; i< _userFlowsJSON.length; i++)
   	  {
   		  var flow = _userFlowsJSON[i];
		     //5.a if same name and cat but different flowNum .. then we are changing the flowNum
   		  if ((flow.name.localeCompare(flowName) == 0) && (flow.flowCat.localeCompare(flowCat) == 0))
			  {
			     if (!confirm("** CAUTION NOT SUPPORTED YET - use REMOVE FLOW**\nChanging flow number from:\n" + flow.flowNum + " -> to:\n" + flowNum ))
				  {
				     alert("** Not Saving **");
				     return;
				  }
				  else
				  {
				     alert("** Not Saving **");
					  return;
				     //override = true;
					  //break;
				  }
				}
   	  }
	  }

	  // if not override, then 2.a ensure flowName not in same category
	  if (!override)
	  {
        for (var i =0; i< _userFlowsJSON.length; i++)
   	  {
   		  var flow = _userFlowsJSON[i];
		     //2.a if same name and cat then not allowed..
   		  if ((flow.name.localeCompare(flowName) == 0) && (flow.flowCat.localeCompare(flowCat) == 0))
			  {
				  alert("Not Saving\nFlowNum (" + flowNum + ") already exists as\nFlowName: " + flow.name + "\nFlowCat: " + flow.flowCat);
				  return;
           }
   	  }
	  }
	} // _flow valid

	//! try to grab thew image..
	if (QRAvatarURL &&  QRAvatarURL.length > 0)
	{
		// start escaping some items..
		//NOTE: I cannot get this to work synchronously..
		try
		{
   	   getImage(QRAvatarURL).then
   		(
   		   function (successurl)
   			{
   			    console.log("Great, Valid Image URL\n\nFYI: Background checking found a VALID QRAvatarURL:\n" + QRAvatarURL + "\nYour Semantic Marker\u00AE works best with an embedded image.");
   			    //alert("Great, Valid Image URL\n\nFYI: Background checking found a VALID QRAvatarURL:\n" + QRAvatarURL + "\nYour Semantic Marker\u00AE works best with an embedded image.");
   			}).catch (function (errorurl)
   			{
   			    alert("Sorry, Invalid Image URL\n\nFYI: Background checking found an INVALID QRAvatarURL:\n" + QRAvatarURL + "\nYou can fix at your convience");
   				 return;
   			})
   	    }
	    catch (error)
	    {
		    alert("INVALID QRAvatarURL: " + QRAvatarURL  + "\nerror: " + error.message);
			 return;
	    }
	}

	alert("Saving Flow\nFlowName: " + flowName + "\nFlowCat: " + flowCat);

//alert("Testing valid write .. no operation taken..");
//return;

	//These are the commands themselves, like change page or feed -- not SemanticMarkers
   var allCommands = document.getElementById(recordedMessagesId).value;
   console.log("all= " + allCommands);

	//NOTE: I think the commands should be as in the end-domain (not SemanticMarkers)
   //allCommands = { "https://iDogWatch.com/bot/cmd/scott@konacurrents.com/p/feed", "https://iDogWatch.com/bot/cmd/scott@konacurrents.com/p/feed"};
   //allCommands = { "https://iDogWatch.com/bot/cmd/USERNAME/PASSWORD/feed", "https://iDogWatch.com/bot/cmd/USERNAME/PASSWORD/feed"};

	// so they have been replaces and encrypted..
	// eg  "https://SemanticMarker.org/bot/sm/saveflow/" + ENCRYPTED_COMMANDS + uuid..;

	//!NOTE: this isn't a message to users, but rather a message to the node-red SemanticMarker backend
	//   /sm/addflow/BASE64 ...
	var allCommandsJSON;


	//BUT the parameters should be  replaced (eg. myName -> USERNAME)
	for (var i=0; i< _SemanticMarkerValues.length; i++)
	{
	   allCommands = allCommands.replaceAll( _SemanticMarkerValues[i],_SemanticMarkerParameters[i]);
   }

	// tack onto these parameters with DEVICE_1, ... DEVICE_n
	// If re-saving, the the flowParameters are valid..
	//var parameters = ["USERNAME","PASSWORD"];
	//var parameters = flowParametersArray;


	//### NOTE: when looking at a flow that was already made generic, the values have already been updated
	//! eg.   scott -> USERNAME,  and others like M5Rainier -> M5_DEVICE_1, ScoobyDoo -> DEVICE_5, DEVICE_3_GEN3
	//! SO .. these should be used in the loop below.. otherwise the parameters are removed..
	//! OR: make parameters the parameters..

	//NOTE: after saving the first time, we don't know the local device (it's already DEVICE_1 for example).
	//Also, before adding say a DEVICE_1, check to see if it already exists, if so .. then DEVICE_2 .. etc

	//! NOW go through the commands looking for the 'devices' anywhere.. if _ALL_ leave it.. but if a parameter name
	//!  THEN: foreach .. replace with DEVICE_1 DEVICE_2  DEVICE_1 (again) .. 
	//! 2.8.24 30K above pacific .. The device names need to be reverse sorted by length (bug Issue #289..)
	//!   Because: name might be M5AtomSocket but there is a device name M5Atom (which matches .. not using delimiter like "/")
	//!   So a solution is to sort the _devicesJSON by length and go from longest to shortest in following loop (or use delimiter)
	//! TODO
	var deviceCount = 0;
	for (var i=0; i< _devicesJSON.length; i++)
	{
       let deviceName = _devicesJSON[i].deviceName;
		 // first see if that name is in our allCommands
	    // JSON == var _devicesJSON =  [{"deviceName":"_GENERIC_"},{"deviceName":"ScoobyDoo"},{"deviceName":"PumpkinUno"} ...
		 // That device name (like scooby) is in our list .. so add. But check for the a unique Device name
		 if (allCommands.indexOf(deviceName) >= 0)
		 {
			 var deviceParamName; // result name, like DEVICE_n where n isn't used yet. (even if M5_DEVICE_1 vs DEVICE_1)
			 var duplicateName = true;
          //! returns a unique deviceCount 
          //! by checking the DEVICE_ + count .. if exists, then return the next number..
			 while (duplicateName)
			 {
				 duplicateName = false;
			    deviceCount++;
			    deviceParamName = "DEVICE_" + deviceCount;
				 for (var i=0; i< parameters.length; i++)
				 {
					 // if name exists, continue..
				    if (parameters[i].indexOf(deviceParamName) >= 0)
					 {
                    duplicateName = true;
					 }
				 }
			 }
			 //! 1.9.24 not using the M5 or GEN3 distinctions, making substitution easier..
			 if (false) 
			 {
			   //! see if M5
			   if (deviceName.indexOf("M5") >= 0)
			      deviceParamName = "M5_" + deviceParamName;
			   else if (deviceName.indexOf("GEN3") >= 0)
			      deviceParamName = deviceParamName + "_GEN3";
				   //added 3.24.23
			   else if (deviceName.indexOf("NO_ONE") >= 0)
			      deviceParamName = "NO_ONE_" + deviceParamName;
			 }
			 else 
			 {
			   if (deviceName.indexOf("NO_ONE") >= 0)
			      deviceParamName = "NO_ONE_" + deviceParamName;
			 }

			 // add name to the parameters array..
			 parameters.push(deviceParamName);
          console.log(" ** Found in allCommands: " + deviceName + " call it: " + deviceParamName);

			 // replace that string everwhere..
	       allCommands = allCommands.replaceAll( deviceName, deviceParamName);

		 }
	}

	//! 7.25.23 group..
	//! NOW go through the commands looking for the 'groups' anywhere.. if _ALL_ leave it.. but if a parameter name
	//!  THEN: foreach .. replace with GORUP_1 GROUP_2  (again) .. 
	var groupCount = 0;
  var groupsArray = _groupsJSON;

  // go through list, but dont include GENERIC and DEFAULT
  for (var i =0; i< groupsArray.length; i++)
  {
		  // key = group name

			  var groupJSON = groupsArray[i];
		     var groupName = groupJSON.group;

		 // first see if that name is in our allCommands
	    // JSON == var _devicesJSON =  [{"deviceName":"_GENERIC_"},{"deviceName":"ScoobyDoo"},{"deviceName":"PumpkinUno"} ...
		 // That device name (like scooby) is in our list .. so add. But check for the a unique Device name
		 if (allCommands.indexOf(groupName) >= 0)
		 {
			 var groupParamName; // result name, like DEVICE_n where n isn't used yet. (even if M5_DEVICE_1 vs DEVICE_1)
			 var duplicateName = true;
          //! returns a unique deviceCount 
          //! by checking the DEVICE_ + count .. if exists, then return the next number..
			 while (duplicateName)
			 {
				 duplicateName = false;
			    groupCount++;
			    groupParamName = "GROUP_" + groupCount;
				 for (var i=0; i< parameters.length; i++)
				 {
					 // if name exists, continue..
				    if (parameters[i].indexOf(groupParamName) >= 0)
					 {
                    duplicateName = true;
					 }
				 }
			 }

			 // add name to the parameters array..
			 parameters.push(groupParamName);
          console.log(" ** Found in allCommands: " + groupName + " call it: " + groupParamName);

			 // replace that string everwhere..
	       allCommands = allCommands.replaceAll( groupName, groupParamName);

		 }

	}

	//! 7.25.23  FLAW: 
	// if the parameter is at the end of the command (eg. feed/GROUP1, vs bot/GROUP/feed the /GROUP_1/ 
	// won't work.. So for GROUP_! .. don't check??

	//NOW: for all parameters, look to see if they are in the flow, if not then they were removed..
	//! Go in reverse so can remove elements in the loop..
	//! https://stackoverflow.com/questions/5767325/how-can-i-remove-a-specific-item-from-an-array
	//!NOTE: here the parameter needs to be enclosed in "/" which assumes not at the end of a command  *** LIMITATION
	for (var i= parameters.length-1; i >= 0; i--)
	{
      // search allCommands (a long string) to see if parameter is still there .. otherwise, remove from array
		var parameter = parameters[i];
		if (parameter.indexOf("GROUP_") >= 0)
		   continue;

		parameter = "/" + parameter + "/";
		// not found == -1
		if (allCommands.indexOf(parameter) < 0)
		{
         console.log("Removing parameter: " + parameter);
			parameters.splice(i, 1);
		}
	}


	//!NOTE: flaw, when a DEVICE_1 and DEVICE_1_GEN3 .. the replace will be 
	//! for all DEVICE_1 with your value .. which will change DEVICE_1_GEN3 to .. name_GEN3
	//!  SO A solution: the parameters are sorted by longest to shortest when being replaced by the next step..


   console.log("parameters = " + parameters);
   console.log("allCommands = " + allCommands);

   // create an array of commands.. in JSON, which will be flow:<allCommandsJSON array>
	var commandsArray = allCommands.split("\n");
	allCommandsJSON = JSON.parse(JSON.stringify(commandsArray));

	var dateString = new Date().toLocaleString();
    // "Friday, Jul 2, 2021"

   document.getElementById(dateTextId).value = dateString;

	//TODO: add the avatar type.. circle/square, but the nodered has to be updated too..

	 // create the parameter and other info..
	 // Currently the parameters are known to this code .. as below
	 var flowMetaJSON = { 
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
							  };

 	 console.log("flowMetaJSON = " + flowMetaJSON);
	 alert(JSON.stringify(flowMetaJSON));

//! setting the testing will NOT save the JSON
var ____testing = false;
if (____testing)
{
   alert("**** JUST TESTING .. NOT SAVING THIS SMART BUTTON ****");
}
else
{
   if (true)
   {
     // sends this JSON as a POST (not a GET). 
	  // When done.. it calls getSMFlows(uuidStringTextId, flowsSelectId);
     sendJSONAsPOST(flowMetaJSON, uuidStringTextId, flowsSelectId, false);

	  // This is needed to let the POST finish.. otherwise the update 
	  // doesn't get results (as the getSMFlows is a call to the same nodered)
     //alert("Finshed update. WIll refresh flows");
   
     //update the flows. Again, a race condition..
     ////getSMFlows(uuidStringTextId, flowsSelectId);
   
     return;
   }
   else
   {
   
   	 // stringify then base64 encode it
   	 flowMetaJSON = JSON.stringify(flowMetaJSON);
   
   	 // convert to BASE64 encoding
       var commandsEncoded = btoa(flowMetaJSON);
   
   
   //TODO .. this needs to be a POST command.. sending the JSON
   
   
       //! send this as a GET command to the bot backend (nodered)
       var webAddress = _semanticMarkerFlowURL + "smflow/saveflow/" + commandsEncoded 
   	    + "?uuid=" + uuid + "&flow=" + flowNum + "&username=" + _username + "&password=" + _password;
   
       console.log("web = " + webAddress);
   
   	 // send this to node-red  "smflow/saveflow"
       justSendGETCommand(webAddress);
   
       //grab the flows
       getSMFlows(uuidStringTextId, flowsSelectId);
   
   }
}

}

//!Remove Flow using info..
function sendRemoveFlow(recordedMessagesId, uuidStringTextId,flowNumberTextId, flowNameTextId, flowCatTextId, privateId, flowsSelectId, isDataId, descTextId, dateTextId, QRAvatarURLTextId, parameterValuesSMId, nextFlowURLId, 
KSMatrixId, dataSMId, artifactsSMId, markupId, languagesSMId, videoSMId, isCircularAvatarId,
audioSMId, locationSMId, inheritedSMId, deckSMId, isMessagingId, isIndirectSMId, indirectSMId,
KSWaveId, bridgeSMId, futureId)
{

   // grab the text.. then wrap the entire things
	var isDataFlag = document.getElementById(isDataId).value;
	var privateflag = document.getElementById(privateId).value;
   var flowNum = document.getElementById(flowNumberTextId).value;
   var flowName = document.getElementById(flowNameTextId).value;
   var flowCat = document.getElementById(flowCatTextId).value;

	var nextFlowURL = document.getElementById(nextFlowURLId).value;

	// 3.22.23 added
	var KSMatrixText = document.getElementById(KSMatrixId).value;
	var dataSM = document.getElementById(dataSMId).value;
	var artifactsSM = document.getElementById(artifactsSMId).value;

	var KSWaveText = document.getElementById(KSWaveId).value;
	var bridgeSM = document.getElementById(bridgeSMId).value;
	var futureText = document.getElementById(futureId).value;
	var languagesSM = document.getElementById(languagesSMId).value;
	var videoSM = document.getElementById(videoSMId).value;
	var isCircularAvatar = document.getElementById(isCircularAvatarId).value;


	// NOTE: multiple lines don't work .. 
   var markupText = document.getElementById(markupId).value;
	// Replace NEWLINE with \n\
   markupText = markupText.replaceAll('\\n','\\n\\');

	var audioSM = document.getElementById(audioSMId).value;
	var locationSM = document.getElementById(locationSMId).value;
	var inheritedSM = document.getElementById(inheritedSMId).value;
	//var isDeck = document.getElementById(isDeckId).value;
	var deckSM = document.getElementById(deckSMId).value;
	var isMessaging = document.getElementById(isMessagingId).value;
	var isIndirectSM = document.getElementById(isIndirectSMId).value;
	var indirectSM = document.getElementById(indirectSMId).value;
	//! end added

	// grab the UUID
   var uuid = document.getElementById(uuidStringTextId).value;
	//require a uuid
	if (!uuid || uuid.length == 0)
	{
      alert("UUID required");
		return;
	} 
	else 
	{
		// check if we own this UUID (if not, then we traversed to it..)
		var validUUID = false;
	   for (let i =0; i< _UUIDNumbers.length; i++)
		{
		   if (_UUIDNumbers[i].localeCompare(uuid)==0)
			{
			   validUUID = true;
				break;
			}
		}
		if (!validUUID)
		{
         alert(uuid + ": Cannot remove a different users SMART Deck." 
		   + " Please inherit and clone to"
		   + " run your own (which is recommended).");
		   return;
		}
	}

	//require a flowNum
	if (!flowNum || flowNum.length == 0)
	{
      alert("flowNumber required");
		return;
	} 

	//require a flowName
	if (!flowName || flowName.length == 0)
	{
      alert("flowName required");
		return;
	} 
	// strip spaces on flowName
   flowName = createValidFlowName(flowName);
   document.getElementById(flowNameTextId).value = flowName;

	//require a flowCat
	if (!flowCat || flowCat.length == 0)
	{
      alert("flow category required");
		return;
	} 
	// strip spaces on flowCat
   flowCat = createValidFlowName(flowCat);
   document.getElementById(flowCatTextId).value = flowCat;


	 // create the parameter and other info..
	 // Currently the parameters are known to this code .. as below
	 var flowMetaJSON = { 
                         "username":_username,
								 "password":_password,
	                      "name" : flowName,
								 "uuid" : uuid,
	                      "flowNum"  : flowNum,
								 "flowCat"  : flowCat
							  };

//	 console.log("flowMetaJSON = " + flowMetaJSON);

	var removeIt = false;
	if (confirm("*** CAUTION *** Removing\nflowName: " + flowName + "\nflowNum: " + flowNum))
	{
	    alert("Removing: " + flowName);
		 removeIt = true;
	}
	else
	{
	    alert("NOT Removing: " + flowName);
	}


   if (removeIt)
   {
     // sends this JSON as a POST (not a GET). 
	  // When done.. it calls getSMFlows(uuidStringTextId, flowsSelectId);
     sendJSONAsPOST(flowMetaJSON, uuidStringTextId, flowsSelectId, true);

	  // This is needed to let the POST finish.. otherwise the update 
	  // doesn't get results (as the getSMFlows is a call to the same nodered)
     //alert("Finshed update. WIll refresh flows");
   
     //update the flows. Again, a race condition..
     ////getSMFlows(uuidStringTextId, flowsSelectId);
   
     return;
   }

} // remove

//!setup a flow recording session
function setupFlowCapture(recordedMessagesId,uuidStringTextId,flowNumberTextId, repeatId, sendOnlyId, flowsSelectId)
{
  // set the modes..
  document.getElementById(recordedMessagesId).innerHTML = "";
  document.getElementById(repeatId).value = "off";
  document.getElementById(sendOnlyId).value = "off";
  _useSemanticMarkerParameters = false;

  //grab the flows
  getSMFlows(uuidStringTextId, flowsSelectId);
}

//! sends FLOW as a Semantic Marker..   BUT this is just the flowMeta info (not the flow)
function sendFlowSM(uuidStringTextId, flowNumberTextId, flowNameTextId, flowCatTextFieldId)
{
   if (!_userFlowsJSON)
	{
      alert("Please click 'Get User SM Flows' to initialize your SMART Buttons");
	   return;
	}
   var flowName = document.getElementById(flowNameTextId).value;
   var flowNum = document.getElementById(flowNumberTextId).value;
   var flowCat = document.getElementById(flowCatTextFieldId).value;

   var uuid = document.getElementById(uuidStringTextId).value;
   var webAddress = _semanticMarkerFlowURL + "smart"
	    + "?uuid=" + uuid + "&flow=" + flowNum +  "&flowname=" + flowName + "&flowcat=" + flowCat;

	// not a parameterized command.. this is straight SemanticMarker.org/bot
   _useSemanticMarkerParameters = false;

	// Now send this..
	// just send SemanticMarker 
	sendSemanticMarkerCommand(webAddress);

}

//! 7.1.23 Dad's 92nd Birthday
//! Sends the JSON of this SMARTButton to the devide specified..
function sendSMARTButtonJSON(uuidStringTextId, flowNumberTextId, flowNameTextId, flowCatTextFieldId)
{
   //format:  {dev:<device>, "SMARTButton": <entire json> }

   if (!_userFlowsJSON)
	{
      alert("Please click 'Get User SM Flows' to initialize your SMART Buttons");
	   return;
	}
   var flowName = document.getElementById(flowNameTextId).value;
   var flowNum = document.getElementById(flowNumberTextId).value;
   var flowCat = document.getElementById(flowCatTextFieldId).value;

   var uuid = document.getElementById(uuidStringTextId).value;

	// grab the flow .. JSON
   let flow = flowOfFlowNum(uuid, flowNum);

	if (!flow)
	{

      alert("Cannot find flow for " + uuid + "." + flowNum);
	   return;
	}

	//!setup the devices
   setupDevice('devices');

	//! check device name..
	if (_isSendToAll)
	{
      alert("Currently only supporting sending SMART Button to single user. Specify a device name");
		return;
	}


	let messageJSON = {"dev":_deviceName,"SMARTButton": flow};

	alert(JSON.stringify(messageJSON));
	
}

//! history of links..   (array of SM)
var _historyOfLinksSM = [];


// pop the stack
function showPreviousLink()
{
	// pop the stack..
   var previousLinkSM = _historyOfLinksSM.pop();

	if (previousLinkSM)
	{
      pushSMLink(previousLinkSM); 
	}
}


//! sets the index of this link..
//! updates the global: _selectedFlowIndex (a hidden value)
//! returns TRUE if valid, and false if can't find (but initiated a web call)
//! Requires that _userFlowsJSON be valid 
//!  if not. then the _cachedFlowsJSON ..
//@returns {boolean}
function setSelectedFlowIndex(thisDeckSM)
{
  let deckSMuuid = getUUIDFromSM(thisDeckSM);
  let deckSMflowNum = getFlowNumFromSM(thisDeckSM);

  let sameUUID = true;
  let validIndex = true;

  // find the flowsSM. Start with singleton _userFlowsJSON 
  for (let i=0; i<_userFlowsJSON.length; i++)
  {
       // find the matching flow..
		 let thisUUID = _userFlowsJSON[i].uuid;
		 if (thisUUID != deckSMuuid)
		 {
		    sameUUID = false;
			 break;
		 }

		 //below is same for this UUID 
		 let thisFlowNum = _userFlowsJSON[i].flowNum;
		 let thisSM = createSM(thisUUID, thisFlowNum);
		 if (thisSM.localeCompare(thisDeckSM)==0)
		 {
           document.getElementById('_selectedFlowIndex').value = i;
			  return true;
	    }
   }
	// didn't find the uuid in the _userFlowsJSON .. so look in cache
	if (!sameUUID)
	{
		// format:  {uuid:u, flows:flowsJSONArray}
      for (let i=0; i<_cachedFlowsJSON.length;i++)
		{
			 var uuid = _cachedFlowsJSON[i].uuid;
			 if (uuid == deckSMuuid)
			 {
		       // swap it out.., then recursively find it..
			    _userFlowsJSON = _cachedFlowsJSON[i].flows;
				 return setSelectedFlowIndex(thisDeckSM);
			 }
		}
	}

	// request a flowName .. which initiates a GET fetch
   flowNameOfSM(thisDeckSM);

	// let the caller know we don't have anything, but check back..
	return false;
}

// pushes the SMLink and calls showSMFlow
function pushSMLink(thisDeckSM)
{
   if (!_userFlowsJSON)
	{
      alert("Please click 'Get User SM Flows' to initialize your SMART Buttons");
	   return;
	}

  // sets the selected in the _userFlowsJSON so the showSMFlow works..
  if (!setSelectedFlowIndex(thisDeckSM))
  {
     alert("Still fetching SMART Button, try again shortly: " + thisDeckSM);
	  return;
  }

  //! 5.1.23 .. updating the global _uuidStringText to this new UUID
  var uuid = getUUIDFromSM(thisDeckSM);
  document.getElementById('_uuidStringText').value = uuid;


	 // 5.1.23 .. done above in the setSelectedFlowIndex
	 //TODO: change UUID..  which requires the flow to change
	 // currently only working with this UUID
	 //now invoke this one..  'flowsSM' is an index into the flows.., replace with our _selectedFlowIndex
showSMFlow('_selectedFlowIndex','parameterValuesSM','_flowValuesSM','_flowNumberText','flowNameText','flowCatText','boolArgValuePrivate','flowSMName','boolArgValueIsData','descriptionText','_recordedMessages','flowDateText','flowQRAvatarURLText','sm1','sm1Command','nextFlowURLText','KSMatrixText', 'dataSMText', 'artifactsSMText', 'markupText','languagesSMText','videoSMText', 'boolArgValueIsCircularAvatar', 'audioSMText', 'locationSMText', 'inheritedSMText',  'deckSMText', 'boolArgValueIsMessaging',  'boolArgValueIsIndirectSM','_indirectSMText', 'KSWaveText', 'bridgeSMText', '_futureText','_QRAvatarId2','otherSM');

}

//! get the SM of the currently shown based on the text values..
//! UUID.flownum
function currentlyShownSM()
{
  var uuid = document.getElementById('_uuidStringText').value;
  var flowNum = document.getElementById('_flowNumberText').value;
  var SM = createSM(uuid, flowNum);
  return SM;
}

//! retrieve based on a SM (so this tears it apart and retrieves it async..)
//! calls flowNameOfFlowNum
function avatarURLOfSM(SM)
{
  let uuid = getUUIDFromSM(SM);
  let flowNum = getFlowNumFromSM(SM);
  if (uuid)
  {
	 return avatarURLOfFlowNum(uuid, flowNum);
  }
  return null;
}

//! retrieve based on a SM (so this tears it apart and retrieves it async..)
//! calls flowNameOfFlowNum
function markupOfSM(SM)
{
  let uuid = getUUIDFromSM(SM);
  let flowNum = getFlowNumFromSM(SM);
  if (uuid)
  {
	 return markupOfFlowNum(uuid, flowNum);
  }
  return null;
}

//! see if this SM.uuid is our UUID, if so then return the flowName
function flowInheritedSMOfSM(SM)
{
   var uuid = getUUIDFromSM(SM);
   var flowNum = getFlowNumFromSM(SM);
   let flow = flowOfFlowNum(uuid, flowNum);
	if (flow)
	{
		console.log("Found cached flowName: " + flow.name);
	   return flow.inheritedSM;
	}
	return null;
}

//! retrieve based on a SM (so this tears it apart and retrieves it async..)
//! calls flowNameOfFlowNum
function flowNameOfSM(SM)
{
  let uuid = getUUIDFromSM(SM);
  let flowNum = getFlowNumFromSM(SM);
  if (uuid)
  {
	 return flowNameOfFlowNum(uuid, flowNum);
  }
  return null;
}

//! see if this SM.uuid is our UUID, if so then return the flowName
//! returns the flow 
function flowOfFlowNum(uuid, flowNum)
{
  // NOW only go through the _cachedFlows
  for (let i=0; i<_cachedFlowsJSON.length;i++)
  {
	  if (uuid == _cachedFlowsJSON[i].uuid)
	  {
		 var userFlows = _cachedFlowsJSON[i].flows;
	    // found match..
		 // look for the same flowNum
		 for (let n=0;n<userFlows.length;n++)
		 {
		   var userFlow = userFlows[n];
		   if (userFlow.flowNum == flowNum)
		   {
				// found and use thie flow name (.name)
		      var flowName = userFlow.name;
				console.log("Found cachedFlow: " + flowName);
		      return userFlow;
		   }
	    }
     }
  }
  // didn't find .. so make a request, but answer won't be back for awhile.

  document.getElementById('_hiddenUUID').value = uuid;
  document.getElementById('_hiddenFlowNum').value = flowNum;

	console.log("Requesting getSMFlows: " + uuid + "." + flowNum);

  //! this should be threadsafe as the calls are synchronous, and the getSMFlows grabs the value 
  //! before it's async call
  getSMFlows('_hiddenUUID', null);

  return null;

}


//! see if this SM.uuid is our UUID, if so then return the flowName
function flowNameOfFlowNum(uuid, flowNum)
{
   let flow = flowOfFlowNum(uuid, flowNum);
	if (flow)
	{
		console.log("Found cached flowName: " + flow.name);
	   return flow.name;
	}
	return null;
}

//! see if this SM.uuid is our UUID, if so then return the flowName
function avatarURLOfFlowNum(uuid, flowNum)
{
	let avatarURL = "https://SemanticMarker.org/images/SM-Circle-R.png";
   let flow = flowOfFlowNum(uuid, flowNum);
	if (flow)
	{
		console.log("Found cached flowName: " + flow.name);

		if (flow.QRAvatarURL && flow.QRAvatarURL.length > 0)
		   avatarURL = flow.QRAvatarURL;
	}
	return avatarURL;

}

//! see if this SM.uuid is our UUID, if so then return the flowName
function markupOfFlowNum(uuid, flowNum)
{
	let markup = "";
   let flow = flowOfFlowNum(uuid, flowNum);
	if (flow)
	{
		console.log("Found cached flowName: " + flow.name);

		if (flow.markup && flow.markup.length > 0)
		   markup = flow.markup;
	}
	return markup;

}

//! 4.30.23 show the SMART Deck, and grab the selected from '_SMARTDeckLinks'
//! Show SM Link
//! This version creates a
function showSMLink(SMARTDeckLinksId)
{
  // get the deckSM from the current links id 
  var thisDeckSM = document.getElementById(SMARTDeckLinksId).value;

  //! call showSMLinkSM
  showSMLinkSM(thisDeckSM);

}

//! 4.30.23 show the SMART Deck, and grab the selected from '_SMARTDeckLinks'
//! Show SM Link
//! This version creates a
function showSMLinkSM(SM)
{
  console.log("showSMLinkSM: " + SM);

  // push which also calls showSMFlow
  //! the currentlyShowSM() is a function that returns the flow shown
  //! It's not pushed on history until we leave it to deeper in the tree
  _historyOfLinksSM.push(currentlyShownSM());

  //now push the link
  pushSMLink(SM);
}

//!creates a SM from uid/flownum
function createSM(uuid, flowNum)
{
   return uuid + "." + flowNum;
}

// 4.30.23 show the deckSM links in a pulldown
// also init the stack
function showSMDeckLinks(SM, deckSMs)
{
  // the deckSM (if available) 
  // modifies _SMARTDeckLinks
  var htmlSelect = "";

  var validLinks = false;
  let deckSMLinks = []; // [{"SM":SM, "flowName":name, "avatarURL:"http link.."}]

		console.log("showSMDeckLinks = " + deckSMs);
		if (deckSMs && deckSMs.length>0)
		{
		  // now parse the deckSMs, comma seperated
		  deckSMs = stripSpaces(deckSMs);

		  //should be list of UUID.flownum, UUID.flownum
		  let deckSMAddresses = deckSMs.split(",");

		  if (deckSMAddresses && deckSMAddresses.length>0)
		  {
			  // the result if itself if no ','
		     for (let i=0; i< deckSMAddresses.length; i++)
			  {
				  validLinks = true;
				  let thisDeckSM = deckSMAddresses[i];

			     // get the single address, 
              let deckSMuuid = getUUIDFromSM(thisDeckSM);
              let deckSMflowNum = getFlowNumFromSM(thisDeckSM);

				  //Add the flowName if this is our UUID
				   var flowName = flowNameOfFlowNum(deckSMuuid, deckSMflowNum);

					// grab the avatar (which will return something..)
					let avatarURL = avatarURLOfFlowNum(deckSMuuid, deckSMflowNum);

					let object = {"SM":thisDeckSM, "flowName":flowName, "avatarURL":avatarURL};
					deckSMLinks.push(object);

					 //don't know the flowName yet ...
               htmlSelect += "<option value='" + thisDeckSM + "'>";
					if (flowName)
					   htmlSelect += flowName + " ";
					htmlSelect += "(" + thisDeckSM + ")</option>";
               htmlSelect += "\n";
				}
			}
		}

  document.getElementById('_SMARTDeckLinks').innerHTML = htmlSelect;

  // 5.2.23 Also update the Hidden aspects
  document.getElementById('_showLinkButton').hidden = !validLinks;;
  document.getElementById('_previousLinkButton').hidden = (_historyOfLinksSM.length == 0);

  //!now update the SMART Button Images
	/*
	 modify document.getElementById("_SMARTButtonsDeckImages").innerHTML
	      <table>
		     <tr>
	          <td><img width="100" src="https://SemanticMarker.org/images/M5/M5Menu.jpg">
	          <td><img width="100" src="https://user-images.githubusercontent.com/5956266/219514302-25c41a8a-f3fd-41de-88fe-be675cb7e5e1.jpeg">
	          <td><img width="100" src="https://user-images.githubusercontent.com/5956266/221418254-a93d02bf-31b4-46fa-b740-bfde510c43aa.jpeg">
	          <td><img width="100" src="https://SemanticMarker.org/users/QHmwUurxC3/LauraSki.jpg">
				</tr>
			</table>
		*/
	let htmlTable = "<table cellspacing='0' cellpadding='4'>\n<tr>";
	// if a previous.. show it
	if (_historyOfLinksSM.length > 0)
	{
		// at(-1) see: https://stackoverflow.com/questions/42501871/peek-operation-in-stack-using-javascript
      let previousSM =  _historyOfLinksSM.at(-1);
      let avatarURL = avatarURLOfSM(previousSM);
      let flowName = flowNameOfSM(previousSM);
		//! https://jkorpela.fi/html/cellborder.html
	   htmlTable += "<td><table border='2' cellpadding='4' ><tr><td bgcolor='lightgrey'>Previous:: " + flowName 
		   + "<br><img width='100' src='" + avatarURL + "' onClick='showPreviousLink()'></td></tr></table></td>\n";
	}
	// use the deckSMLinks array created above .. where the avatarURL is set
	for (let i=0; i< deckSMLinks.length; i++)
	{
	   let object = deckSMLinks[i];
		let thisSM = object.SM;
		let flowName = object.flowName;
		let avatarURL = object.avatarURL;
	   htmlTable += "<td>" + flowName + "<br><img width='100' src='" + avatarURL + "' onClick='showSMLinkSM(\"" + thisSM + "\")'></td>\n";
	}

	// 5.6.23 if the inherited .. tack the button at the end
   let inheritedSM = flowInheritedSMOfSM(SM);
	if (inheritedSM && inheritedSM.length > 0)
	{
	  // tack on inherited at the end..
	  let flowName = flowNameOfSM(inheritedSM);
	  let avatarURL = avatarURLOfSM(inheritedSM);
	   htmlTable += "<td><table border='2' cellpadding='4' ><tr><td bgcolor='grey'>1.Inherited:: " + flowName 
		   + "<br><img width='100' src='" + avatarURL + "' onClick='showSMLinkSM(\"" + inheritedSM + "\")'></td>\n";
	}


	htmlTable += "</tr>\n</table>\n";
	document.getElementById("_SMARTButtonsDeckImages").innerHTML = htmlTable;

	console.log(htmlTable);

} // showSMDeckLinks(SM, deckSMs)

// 5.10.23, (4.30.23 show the deckSM links in a pulldown)
// return an array of the deckSM  links
// [{"SM":SM, "flowName":name, "avatarURL:"http link.."}]
//function getSMDeckLinksArray(SM, deckSMs)
function getSMDeckLinksArray(deckSMs)
{
  // the deckSM (if available) 
  var htmlSelect = "";

  var validLinks = false;
  let deckSMLinks = []; // [{"SM":SM, "flowName":name, "avatarURL:"http link.."}]

		if (deckSMs && deckSMs.length>0)
		{
		  // now parse the deckSMs, comma seperated
		  deckSMs = stripSpaces(deckSMs);

		  //should be list of UUID.flownum, UUID.flownum
		  let deckSMAddresses = deckSMs.split(",");

		  if (deckSMAddresses && deckSMAddresses.length>0)
		  {
			  // the result if itself if no ','
		     for (let i=0; i< deckSMAddresses.length; i++)
			  {
				  validLinks = true;
				  let thisDeckSM = deckSMAddresses[i];

			     // get the single address, 
              let deckSMuuid = getUUIDFromSM(thisDeckSM);
              let deckSMflowNum = getFlowNumFromSM(thisDeckSM);

				  //Add the flowName if this is our UUID
				   var flowName = flowNameOfFlowNum(deckSMuuid, deckSMflowNum);

					// grab the avatar (which will return something..)
					let avatarURL = avatarURLOfFlowNum(deckSMuuid, deckSMflowNum);

					let object = {"SM":thisDeckSM, "flowName":flowName, "avatarURL":avatarURL};
					deckSMLinks.push(object);

				}
			}
		}

   return deckSMLinks;

} // getSMDeckLinks(SM, deckSMs)

//!copy of the user flow
var _userFlowsJSON = null;

/**
* Show SM Flow (using _userFlowsJSON global)
* @param {flowsSelectId} id of flowsSelect
* @param {flowsParameterId} id of parameterSelect
* @param {flowsFlowId} id of flowsSelect
* @param {flowNumId} id of flowNum
* @param {flowNameId} id of flowName
* @param {flowCatId} id of flow Category
* @param {flowPrivateId} id of flowIsPrivate
* @param {flowSMNameId} id of flowSMName the Semantic Marker&#x2122; address
* @param {flowIsDataId} id of flowIsData
* @param {descTextId} id of description
* @param {recordedTextId} id of recordedText area. If non null, then decode the flow into HTTP calls in this area
* @param {dateTextId} id of date field
* @param {QRAvatarURLTextId} id of QRAvatarURLTextId field
* @param {sm} id of where the SM will go field  (if NULL then don't create SM)
* @param {smCommand} id of where the SM command will go field. These will be escaped..
* @param {nextFlowURLId} id of next FLOW URL (a string, maybe multiple later..) 1.29.23
*/
function showSMFlow(flowsSelectId, flowsParameterId, flowsFlowId, flowNumId, flowNameId, flowCatId, flowPrivateId, flowSMNameId, flowIsDataId, descTextId, recordedTextId, dateTextId, QRAvatarURLTextId, sm, smCommand, nextFlowURLId, KSMatrixId, dataSMId, artifactsSMId, markupId, languagesSMId, videoSMId, isCircularAvatarId,
audioSMId, locationSMId, inheritedSMId,  deckSMId, isMessagingId, isIndirectSMId, indirectSMId,
KSWaveId, bridgeSMId, futureId, QRAvatarId, otherSMId)
{
//TODO:: Figure out the flowsSelectedId  .. I think it's somewhere already..
  // use global id:  
  var selectedFlowIndex = document.getElementById(flowsSelectId).value;
  var flow = _userFlowsJSON[selectedFlowIndex];
  var flowName = flow.name;
  var private = flow.private;
  var isdata = flow.isdata;
  var uuid = flow.uuid;
  var flowNum = flow.flowNum;
  var flowCat = flow.flowCat;
  var desc = flow.desc;
  var date = flow.date;
  var QRAvatarURL = flow.QRAvatarURL;
  var nextFlowURL = flow.nextFlowURL;

  var n1 = document.getElementById(flowNameId);
  var n2 = document.getElementById(flowCatId);
  var n3 = document.getElementById(dateTextId);

  document.getElementById(flowCatId).value = flowCat;
  document.getElementById(flowNumId).value = flowNum;
  document.getElementById(flowNameId).value = flowName;
  document.getElementById(flowPrivateId).value = private;
  document.getElementById(flowIsDataId).value = isdata;
  document.getElementById(descTextId).value = desc;
  document.getElementById(dateTextId).value = date;
  document.getElementById(QRAvatarURLTextId).value = QRAvatarURL;
  document.getElementById(nextFlowURLId).value = nextFlowURL;

	// 3.22.23 added
	var KSWaveText = flow.KSWave?flow.KSWave:"";
	var bridgeSM = flow.bridgeSM?flow.bridgeSM:"";
	// it's flow.future (not futureText)
	var futureText = flow.future?flow.future:"";
	var languagesSM = flow.languagesSM?flow.languagesSM:"";
	var videoSM = flow.videoSM?flow.videoSM:"";
	var isCircularAvatar = flow.isCircularAvatar?flow.isCircularAvatar:"false";

	var dataSM = flow.dataSM?flow.dataSM:"";
	var artifactsSM = flow.artifactsSM?flow.artifactsSM:"";
	var KSMatrix = flow.KSMatrix?flow.KSMatrix:"";
	var markup = flow.markup?flow.markup:"";

	var audioSM = flow.audioSM?flow.audioSM:"";
	var locationSM = flow.locationSM?flow.locationSM:"";
	var inheritedSM = flow.inheritedSM?flow.inheritedSM:"";
	//var isDeck = flow.isDeck?flow.isDeck:"false";
	var deckSM = flow.deckSM?flow.deckSM:"";
	var isMessaging = flow.isMessage?flow.isMessaging:"false";
	var isIndirectSM = flow.isIndirectSM?flow.isIndirectSM:"false";
	var indirectSM = flow.indirectSM?flow.indirectSM:"";

	// 3.22.23 added
	 //3.24.23
	 document.getElementById(KSWaveId).value = KSWaveText;
	 document.getElementById(bridgeSMId).value = bridgeSM;
	 document.getElementById(futureId).value = futureText;
	 document.getElementById(languagesSMId).value = languagesSM;
	 document.getElementById(videoSMId).value = videoSM;
	 document.getElementById(isCircularAvatarId).value = isCircularAvatar;

	 document.getElementById(KSMatrixId).value = KSMatrix;
	 document.getElementById(dataSMId).value = dataSM;
	 document.getElementById(artifactsSMId).value = artifactsSM;
	 document.getElementById(markupId).value = markup;

	 document.getElementById(audioSMId).value = audioSM;
	 document.getElementById(locationSMId).value = locationSM;
	 document.getElementById(inheritedSMId).value = inheritedSM;
	 //document.getElementById(isDeckId).value = isDeck;
	 document.getElementById(deckSMId).value = deckSM;
	 document.getElementById(isMessagingId).value = isMessaging;
	 document.getElementById(isIndirectSMId).value = isIndirectSM;
	 document.getElementById(indirectSMId).value = indirectSM;

	 //! 5.7.23 (year since Amber grad L&C
	 let o = document.getElementById('_SMARTMarkup');
	 //! look for @(UUID.flow), @(audio), @(video), @(SMARTDeck) and convert..  @(playlist), @(QRAvatar), @(SM) ..etc
    markup = replaceSMLinks(markup, flow, true);
	 o.innerHTML = markup;

	 // 4.30.23 show the deckSM links in a pulldown
	 // let it know what the parent starting this..
    showSMDeckLinks( createSM(uuid, flowNum), deckSM);


  // set semantic marker
  var webAddress = _semanticMarkerFlowURL + "smart"
	    + "?uuid=" + uuid + "&flow=" + flowNum + "&flowname=" + flowName + "&flowcat=" + flowCat;
  document.getElementById(flowSMNameId).value = webAddress;

  var parametersArray = flow.parameters;
  var flowArray = flow.flow;


  //! 5.8.23 add this (without passing a parameter)
  //! update _mySMLink
  var mySM = createSM(uuid, flowNum);
  document.getElementById('_mySMLink').value = mySM;

  //TODO: SORT..

  //then set the parameter 
		var flowsHTML = "";
      for (var i =0; i< parametersArray.length; i++)
		{
			var element = parametersArray[i];

			// onselect call showSMFlow() but then it checks value .. (can be same value)
			flowsHTML += "<option value='" + i + "'>" + element + "</option>\n";

			//Also tack on to the web address
			webAddress += "&" + element + "=";
		}

		//test.. works.. the 
		if (false)
		{
			 //escape the element
			 // SEE: https://stackoverflow.com/questions/332872/encode-url-in-javascript
		    var elementValue = "This is a long string";
			 elementValue = encodeURIComponent(elementValue);

			 var element = "LONG_NAME";

			 //Also tack on to the web address
			 webAddress += "&" + element + "=" + elementValue;

//TODO:  OR THESE ARE added in the flow description which is grabbed by the Custom Flow page..
// 1.19.23
      }


		if (flowsHTML.length > 0)
         document.getElementById(flowsParameterId).innerHTML = flowsHTML;

  //then set the and flowsID
		flowsHTML = "";
      for (var i =0; i< flowArray.length; i++)
		{
			var element = flowArray[i];
			flowsHTML += "<option value='" + i + "'>" + element + "</option>\n";
		}

		if (flowsHTML.length > 0)
         document.getElementById(flowsFlowId).innerHTML = flowsHTML;

		//! now draw the semantic marker (sm1command is where SM address is placed..)
	   var avatarURL;
		if (QRAvatarURL && QRAvatarURL.length > 0)
		   avatarURL = QRAvatarURL;
		else
		   avatarURL = "https://SemanticMarker.org/images/SM-Circle-R.png";

		// create a semantic marker with the avatarURL (since this is called 2 times for same 'sm' .. only do it once)
		if (sm)
         createSemanticMarkerPath(webAddress, avatarURL, sm, smCommand);

		if (otherSMId)
         createSemanticMarkerPath(webAddress, avatarURL, otherSMId, smCommand);

		 // set the Avatar URL
       var q = document.getElementById(QRAvatarId);
		 q.src = avatarURL;

		//12.8.22
		if (recordedTextId)
		{
		   // break up the flow
		   flowsHTML = "";
         for (var i =0; i< flowArray.length; i++)
		   {
			   var element = flowArray[i];

			   flowsHTML += element + "\n";
		   }

		   if (flowsHTML.length > 0)
            document.getElementById(recordedTextId).value = flowsHTML;

		}
}

//! 5.8.23 Pauls 69th birthday
//! hide or show the markup (and later other) fields
function hideShowMarkup(elementIdArray)
{
	 var doHide = false;
	 for (let i=0; i< elementIdArray.length; i++)
	 {
	    var elementId = elementIdArray[i];
	    var o = document.getElementById(elementIdArray[i]);
		 if (o)
		 {
		    doHide = ! o.hidden;
			 break;
		 }
	 }
	 for (let i=0; i< elementIdArray.length; i++)
	 {
	   var elementId = elementIdArray[i];
	   var o = document.getElementById(elementId);
		if (o)
	     o.hidden = doHide;

	 }

	 //save state in a cookie
	 saveCookies();


}

//! 5.19.23 use a tidy HTML function
//! 
function tidy(html)
{
  var d = document.createElement('div');
  d.innerHTML = html;
  return d.innerHTML;
}

//! 6.1.23 call to get the SemanticMarker (generic with SM) from the _base64Images array
//! SM in form UUID.flow
function getBase64Image(SM)
{

  var img = null;
  for (var i =0; i< _base64Images.length; i++)
  {
	  // smflow is a JSON object {name, ...}
     let object = _base64Images[i];
     let SM = object.SM;
	  if (SM.localeCompare(object.SM)==0)
	  {
	     //found match .. could still be nil
	     let img = object.img;
		  return img;
	  }
  }
  return img;
}

//! 5.7.23 (Abandoned house burning down at corner..)
//! pass in the markup text, and return same with links..
//! 5.19.23 .. syntax:   @SM(uuid.flow)  and other @u(u.f) @web(dkkdkd)
//! want to grab items from the SMART button, like the avatar, the next links.. the Audio..
//! isInternalWeb === this browser, false == remote web    TODO..
//! 5.22.23 the PDF created won't have live video and won't have a link .. so add it as a button..
//! 5.29.23 add @(playlist),  @(header), @(avatar), @(QRAvatar), @(SM) .. todo
function replaceSMLinks(markup, flow, isInternalWeb)
{
	 let markupFinal = "";
	 for (let i=0; i< markup.length-1; i++)
	 {
		 let c = markup[i];
		 let c2 = markup[i+1];
		 // @(  .. not many HTML that use that .. so not worring..
	    if (c == '@' && c2 == '(')
		 {
		    i += 2;
			 // @(uuid.flow),  @(video) @(audio) ...
		    // now look for @( <internal> )
			 
          // search for ) .. or end of file
			 let parsedToken = "";
			 while (markup[i] != ')'  && i < markup.length)
			 {
			    parsedToken += markup[i];
				 i++;
			 }
			 // if not a ) then hit the end of the line.. give up..
			 if (markup[i] != ')')
			 {
				 // note i could be reversed and continue loop..
			    console.log("*** Invalid HTML syntax***");
				 return tidy(markup);
			 }
    
		     //! get the flowName, but it might be null since
		     //! async call (if not cached) Call back later..
	        let flowName = flow.name; // flowName (but name in JSON0

		     //! get the markup
		     let videoSM = flow.videoSM;
		     let QRAvatarURL = flow.QRAvatarURL;
		     let audioSM = flow.audioSM;
           let uuid = flow.uuid;
	        let flowNum = flow.flowNum;
	        let nextURL = flow.nextFlowURL;
			  let desc = flow.desc;
			  //etc..

   		 if (parsedToken.localeCompare("header")==0)
   		 {
				 let SM = createSM(uuid,flowNum);
      		 let webAddress = _semanticMarkerFlowURL  + "smart?uuid=" + uuid + "&flow=" + flowNum;
				 let result = "<h2>Semantic Marker&#x2122; SMART Button</h2>\n";
      
      		 result += "<h3 id='" + SM + "'>";
      		 if (flowName)
      		    result += flowName;
             result += " (" + SM + ")\n";
      
      		 result += "</h3>\n";
             result += "<a href='" + webAddress + "'>" + "(Invoke SMART Button - or touch Semantic Marker&#x2122;)" + "</a><br>\n" ;
      		 result += "<p>" + desc + "</p><br>\n";

				 //append to result
				 markupFinal += result;
   
   		 }
			 else if (parsedToken.localeCompare("video")==0)
			 {
			       if (videoSM && videoSM.length > 0)
					 {
						 if (false)
						 {
							// This version just creates an Image with the Play button
					      let mediaName = "video";
					      let mediaType = "video/mp4";
					      let showVideo = "<" + mediaName + " width='480' height='320' controls><source src='" + videoSM + "' type='" + mediaType + "'>"
   			         markupFinal += "<br>" + showVideo + "</video> ";
						 }
						 else
						 {
							// This version plays automatically
						   let showVideo = "<img width='50%' src='" + videoSM + "'>";
   			         markupFinal += "<br>" + showVideo + "<br>";

                     //! 5.22.23 the PDF created won't have live video and won't have a link .. so add it as a button..
							//!  <a href="videoURL ..">videoURL</a>
							showVideo = "<a href='" + videoSM + "'>Play Video</a><br>";
   			         markupFinal += "<br>" + showVideo + "<br>";

						 }
					 }
			 }
			 else if (parsedToken.localeCompare("audio")==0)
			 {
			       if (audioSM && audioSM.length > 0)
					 {
   			       let mediaName = "audio";
                   let mediaType = "audio/mp3";
					    let showAudio = "<" + mediaName + " width='480' height='320' controls><source src='" + audioSM + "' type='" + mediaType + "'>"
   			       markupFinal += "<br>" + showAudio + "</audio> ";
					 }
			 }
			 //! @(playlist) pulls out the playlist in 'artifactSM' for now...
			 else if (parsedToken.localeCompare("playlist")==0)
			 {
			       var playlist = flow.artifactsSM;
   		       markupFinal += "<br>" + processPlaylist(playlist); 
			 }
			 //! @(slideshow) pulls out the slideshow in 'artifactSM' for now...
			 else if (parsedToken.localeCompare("slideshow")==0)
			 {
			       var slideshow = flow.artifactsSM;
   		       markupFinal += "<br>" + processSlideshow(slideshow); 
			 }
			 // create a table for the SMART Deck.. taken from the code below
			 else if (parsedToken.localeCompare("SMARTDeck")==0)
			 {
       		//! #### The table for the other avatars
      	    // 4.30.23 show the deckSM links in a pulldown
      	    // let it know what the parent starting this..
             let deckSMLinks = getSMDeckLinksArray(flow.deckSM);
       		 let htmlTable = "<br><b>SMART Deck</b><table>";
            	// use the deckSMLinks array created above .. where the avatarURL is set
            	for (let i=0; i< deckSMLinks.length; i++)
            	{
            	   let object = deckSMLinks[i];
            		let thisSM = object.SM;
            		let flowName = object.flowName;
            		let avatarURL = object.avatarURL;

						// grabs the image .. which should already be retrieved
						// this will be base64 image .. with lots of text..
						let semanticMarkerImageHTML = getBase64Image(object.SM);
						if (semanticMarkerImageHTML)
						{

							// note the semanticMarkerHTML ==  the <img> already..
							//<img width=200 src="data:image/png;base64,${msg.merged.toString('base64')}">;

						   // the href=#SM is a link inside this document
            	      htmlTable += "<td style='vertical-align:top'>" + flowName + "<br><a href='#" + thisSM + "'>" +  semanticMarkerImageHTML + "</a>\n";
            	      htmlTable += "<br><a href='#" + thisSM + "'><img width='200' src='" + avatarURL + "' onClick='showSMLinkSM(\"" + thisSM + "\")'></a></td>\n";

						}
						else
						{
      	            htmlTable += "<td>" + flowName + "<br><a href='#" + thisSM + "'><img width='100' src='" + avatarURL + "' onClick='showSMLinkSM(\"" + thisSM + "\")'></a></td>\n";
						}
            	}
            
				//.... doesn't seem to be doing this code.. it's 0:Inherited instead

            	// 5.6.23 if the inherited .. tack the button at the end
               let inheritedSM = flow.inheritedSM;
            	if (inheritedSM && inheritedSM.length > 0)
            	{
            	  // tack on inherited at the end..
            	  let flowName = flowNameOfSM(inheritedSM);
            	  let avatarURL = avatarURLOfSM(inheritedSM);

						// grabs the image .. which should already be retrieved
						// this will be base64 image .. with lots of text..
						let semanticMarkerImageHTML = getBase64Image(inheritedSM);
						if (semanticMarkerImageHTML)
						{

						   // the href=#SM is a link inside this document

            	      htmlTable += "<td><table border='2' cellpadding='4' ><tr style='vertical-align:top'><td bgcolor='grey'>2.Inherited:: " + flowName 
							+ "<br><a href='#" + inheritedSM + "'>" +  semanticMarkerImageHTML + "</a>\n"
            		   + "<br><img width='200' src='" + avatarURL + "' onClick='showSMLinkSM(\"" + inheritedSM + "\")'></a></td>\n";

						}
						else
						{
            	      htmlTable += "<td><table border='2' cellpadding='4' ><tr><td bgcolor='grey'>3.Inherited:: " + flowName 
         		   + "<br><a href='#" + inheritedSM + "'><img width='100' src='" + avatarURL + "' onClick='showSMLinkSM(\"" + inheritedSM + "\")'></a></td>\n";
					   }
            	}
            
            
            	htmlTable += "</tr>\n</table><br>\n";

					console.log(htmlTable);
      
      			markupFinal += htmlTable;
			 }
			 else if (parsedToken.localeCompare("QRAvatar")==0 || parsedToken.localeCompare("avatar")==0)
			 {
   			    markupFinal += "<br><img width='380' src='" + QRAvatarURL + "'>\n";
			 }
			 else if (parsedToken.localeCompare("SM")==0)
			 {
   			    //markupFinal += "<br><img width='380' src='" + QRAvatarURL + "'>\n";
					 //TODO... how to create one on the fly ..?? when the HTML won't have this .. 
					 //your SM.. but it would be nice to show other SM's too..
			 }
			 else if (parsedToken.localeCompare("next")==0)
			 {
			    // add a next pointer.. using URL ..
	           if (nextURL  && nextURL.length > 0 && "undefined".localeCompare(nextURL)!=0)
				  {
				    let nextButton = "<br><a href='" + nextURL +  "'>" + nextURL + "</a>";
   			    markupFinal += nextButton + " ";
				  }
			 }
			 else
			 {
   			 //parsedToken == our SM, lets add  push

				 let thisSMUUID = getUUIDFromSM(parsedToken);
             let thisSMflowNum = getFlowNumFromSM(parsedToken);
             let thisSMFlow = flowOfFlowNum(thisSMUUID, thisSMflowNum);
				 // see if this has been retrieved..
				 //NOTE: potential for a named item .. that is changed later after the async retrieval
				 //Need to try promise: https://www.geeksforgeeks.org/how-to-make-javascript-wait-for-a-api-request-to-return/
				 //TODO..
				 let thisFlowName = thisSMFlow?thisSMFlow.name:parsedToken;

   			 // quote the SM 
				 let showButton = "";
				 // internal is the SMART Button page
				 // external is the web page @(markup)
				 if (isInternalWeb)
   			    showButton = "<input type='button' onclick=\"showSMLinkSM('" + parsedToken + "')\" value='" + thisFlowName + "'/>";
				 else
				    showButton = "<a href='" + _semanticMarkerFlowURL  + "smart?" + thisSMUUID + "&flow=" + thisSMflowNum + "'>" + thisFlowName + "</a>";
   			 markupFinal += showButton + " ";
			 }
		 }
		 else
		    markupFinal += c;

		 // next loop will have c being the c2 spot if no match.
		 // i == last character read (c2 or the ')'
		 // and it's incremented top of this loop

	 }

	 // tidy up the HTML..
	 markupFinal = tidy(markupFinal);
	 console.log("MarkupFINAL = "  + markupFinal);
	 return markupFinal;

}

// sorted by {"uuid":UUID, "flows":JSONFlow}
//NOTE: this might be a subset of the flows (in case when just asking for a single flow)
var _cachedFlowsJSON = [];

//! gets or creates a UUID for existing user "/createuuid"
//! Makes async request for the JSON data
//! NOTE: if flowsSelectId == null then don't update the global, only the cache
function getSMFlows(uuidStringTextId, flowsSelectId)
{
  // the means this is just a retrieval to update the _cache
  let onlyGetFlows = flowsSelectId == null;

  let uuid;
  if (onlyGetFlows)
  {
     uuid = document.getElementById(uuidStringTextId).value;
  }
  else
  {

     // get the selected from the _UUID_Select options, which will be a UUID
     var uuidSelect = document.getElementById('_UUID_Select');
     var uuidSelected = uuidSelect.value;


     //new: 
     uuid = uuidSelected;

     //now update the uuidStringTextId
     document.getElementById(uuidStringTextId).value = uuid;
	}
  
  //TODO: when run as an unnamed user, the username/password isn't needed (but the user can't update)

  // async command
  var command = _semanticMarkerFlowURL + "smart?uuid=" + uuid + "&username=" + _username + "&password=" + _password  + "&format=json";

  var xhttp = new XMLHttpRequest();

  // format=json so this will be json..
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
		//!result is the SM FLOW flows.. [{flow},{flow}]
		var jsonReply = this.responseText;

		if (jsonReply &&  jsonReply.indexOf("error") >= 0)
		{

         alert(jsonReply);
			return;
		}
		if (jsonReply.length == 0 )
		{

         alert("You must login to your agent");
			return;
		}

      var flowsJSON = JSON.parse(jsonReply);

		// 4.30.23
		// push this object, or update the one there..
		var object = {"uuid":uuid,"flows":flowsJSON};
		var found = false;
		for (let i=0; i< _cachedFlowsJSON.length;i++)
		{
		   if (uuid == _cachedFlowsJSON[i].uuid)
			{
				console.log("pushing (updating cashedFlow): " + _cachedFlowsJSON[i].uuid);
				// update the cachedFlow
            _cachedFlowsJSON[i].flows = flowsJSON;
				found = true;
				break;
			}
		}
		// adding a new cachedFlowJSON object
		if (!found)
		{
			console.log("pushing cashedFlow: " + object.uuid);
		   _cachedFlowsJSON.push(object);
		}

		// 4.30.23 .. if null then we are just requesting the UUID, for use by other flowss
		if (onlyGetFlows)
		   return;

		//save globally for the current showSMFlow
      _userFlowsJSON = flowsJSON;

		console.log("getSMFlows updating result _userFlowsJSON");

		var flowsHTML = "";

		//!Try sorting by Category, then Num
		//! https://stackoverflow.com/questions/4833651/javascript-array-sort-and-unique
		//! First find the categories..
		var categories = [];
		var flowNumbers = [];
		var flowNames = [];
      for (var i =0; i< flowsJSON.length; i++)
		{
			var flow = flowsJSON[i];
			categories.push(flow.flowCat);
			flowNumbers.push(flow.flowNum);
			flowNames.push(flow.name);
		}
		//!alphebet sort
		categories.sort();

		//!alphebet sort
		flowNames.sort();

		//! sort numbers so 10 and 100 are in right order (Actually not used as the NAME is the sort)
		//! https://stackoverflow.com/questions/1063007/how-to-sort-an-array-of-integers-correctly
		flowNumbers.sort(function(a,b) { return a-b; });

		// now make them unique (new Set) and back to an array
		categories = Array.from(new Set(categories));
		flowNumbers = Array.from(new Set(flowNumbers));
		flowNames = Array.from(new Set(flowNames));

		//! groups: https://stackoverflow.com/questions/17316540/make-some-options-in-a-select-menu-unselectable
		for (flowCat of categories)
		{
			flowsHTML += "<optgroup label='" + flowCat + "'>" + "\n";

			for (flowName of flowNames)
			{
            for (var i =0; i< flowsJSON.length; i++)
		      {
			      var flow = flowsJSON[i];
					thisFlowName = flow.name;
					thisFlowCat = flow.flowCat;
					thisFlowNum = flow.flowNum;

					// only continue if same flow category
					if (thisFlowCat.localeCompare(flowCat) != 0)
					   continue;

					// only continue if same flow name
					if (thisFlowName.localeCompare(flowName) != 0)
					   continue;

					// now we are the correct flowCategory, and flowName
		         for (flowNum of flowNumbers)
				   {
					   if (thisFlowNum.localeCompare(flowNum) != 0)
						   continue;

						// Finally output an option
			         flowsHTML += "<option value='" + i + "'>" + flow.name + "</option>\n";
			         //flowsHTML += "<option value='" + i + "'>" + flow.name + " (" + flow.flowNum + ")</option>\n";
   
				   }
				}

			}
			flowsHTML += "</optgroup> \n";

		}

		// if no flowsSelectId
		if (flowsHTML.length > 0 && flowsSelectId)
		{
         document.getElementById(flowsSelectId).innerHTML = flowsHTML;

//ISSUE: grabbing a different UUID .. this id will be wrong..
		   // set the first option.. BUT this doesn't fill in the values..
         document.getElementById(flowsSelectId).value = 0;
		}

    }
  };
  xhttp.open("GET",command, true);
  xhttp.send();
}

//! called to get just the parameters of a flow's category
//! flowCatDupID .. is the other Id where the flow is shown
function getCatParms(flowsSelectId, flowsParameterId, flowCatDupId)
{
  // use global id:  
  var selectedFlowIndex = document.getElementById(flowsSelectId).value;
  var flow = _userFlowsJSON[selectedFlowIndex];

  //! grab values from the JSOn flow object 
  var flowName = flow.name;
  var private = flow.private;
  var isdata = flow.isdata;
  var uuid = flow.uuid;
  var flowNum = flow.flowNum;
  var flowCat = flow.flowCat;
  var date = flow.date;


  //! update the duplicate flow category text iD
  document.getElementById(flowCatDupId).value = flowCat;

  //NOTE this will return all the flows just in that cat (TODO).
  var command = _semanticMarkerFlowURL + "smart?uuid=" + uuid + "&username="  + "&flowcat=" + flowCat;

  var xhttp = new XMLHttpRequest();

  // format=json so this will be json..
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
		//!result is the SM FLOW flows.. [{flow},{flow}]
		var jsonReply = this.responseText;

		// array of flows
      var flowsJSON = JSON.parse(jsonReply);


		var flowsHTML = "";
		// for now... since no flownum specified, we have all the flows..
		// SO we have the info to sort by this category ourselves...
		// Eventually, this query will return only those in the category..
		var uniqueParametersArray = [];

      for (var x =0; x< flowsJSON.length; x++)
		{
			var flow = flowsJSON[x];
		   parametersArray = flow.parameters;

			// only for this category
			var category = flow.flowCat;
			if (!category || !flowCat || category.localeCompare(flowCat)!=0)
			   continue;


         //then set the parameter 
         for (var i =0; i< parametersArray.length; i++)
		   {
			   var element = parametersArray[i];

				//check if exists already..
				if (uniqueParametersArray.indexOf(element) >= 0)
				   continue;

				//add otherwise
				uniqueParametersArray.push(element);


			   flowsHTML += "<option value='" + i + "'>" + element + "</option>\n";
		   }
		}

		if (flowsHTML.length > 0)
         document.getElementById(flowsParameterId).innerHTML = flowsHTML;

    } // this.readyState
  };
  xhttp.open("GET",command, true);
  xhttp.send();
}


//!test the POST
function sendJSONAsPOST(flowMetaJSON, uuidStringTextId, flowsSelectId, removeFlow)
{

	 /*
	 var flowMetaJSON = { "name" : "scotty",
								 "uuid" : "uuid",
								 "isdata": "false",
								 "private": "true", 
	                      "flowNum"  : "1",
								 "flowCat"  : "maggie",
								 "date" : "DATE"
								};

*/

    let xhr = new XMLHttpRequest();
    //xhr.open("POST", "http://localhost:1880/smflowpost");

	 if (removeFlow)
       xhr.open("POST", _semanticMarkerFlowURL + "smflowpostremove");
	 else
       xhr.open("POST", _semanticMarkerFlowURL + "smflowpost");

    xhr.setRequestHeader("Accept", "application/json");
    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
      if (xhr.readyState === 4) {
        console.log(xhr.status);
        console.log(xhr.responseText);

		  // call again and get the updated JSON flow (BUT THis is just one UUID ..)
        getSMFlows(uuidStringTextId, flowsSelectId);
      }};


    xhr.send(JSON.stringify(flowMetaJSON));
}


//!test the POST, Only send username and password
function repostToHidePassword()
{
//NOT WORKING.. can't figure out getting past error of duplicate symbols
//https://developer.mozilla.org/en-US/docs/Web/API/Document/write
// Seems document.write() not liked 

	// https://stackoverflow.com/questions/483745/replace-html-page-with-contents-retrieved-via-ajax
   // seems it trys to create duplicate variable '_useFinal'

	 var flowMetaJSON = { "username" : _username,
								 "password" : _password
								};


    let xhr = new XMLHttpRequest();
    //xhr.open("POST", "http://localhost:1880/posttest");
    xhr.open("POST", _semanticMarkerFlowURL + "userpagemessagesPOST");

    xhr.setRequestHeader("Accept", "application/json");
    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
      if (xhr.readyState === 4) {
        console.log(xhr.status);
        //console.log(xhr.responseText);
		  //document.querySelector('html').innerHTML = xhr.responseText;
		  document.open();
        document.write(xhr.responseText);
        document.close();
      }};

    console.log(JSON.stringify(flowMetaJSON));

    xhr.send(JSON.stringify(flowMetaJSON));
}

//! store the uuid, flow of the copied SM
// {"SM":sm, "uuid":uuid, "flowNum":flownum, "flow":flow};
var _lastCopiedSM = null;

//! inherits from the copied SM, user still has to save
//!  Inherit SM
function inheritSM(justClone)
{
   if (!_userFlowsJSON)
	{
      alert("Please click 'Get User SM Flows' to initialize your SMART Buttons");
	   return;
	}
   // dont' use the UUID..
	if (!_lastCopiedSM)
	{
      alert("Please click 'Copy SM' first before the Inherit button");
		return;
	}
	var uuid = _lastCopiedSM.uuid;
	var flowNum = _lastCopiedSM.flowNum;
	if (justClone)
	{
	   // use a unique FlowNum
		flowNum = createFlowNumber();
	}
	var flow = _lastCopiedSM.flow;
	if (flow)
	{
	  //! check if flowNum already used .. in which case the user must choose a different SMART Deck (uuid)
	  for (let i=0; i< _userFlowsJSON.length; i++)
	  {
	      var num = _userFlowsJSON[i].flowNum;
			if (num == flowNum)
			{
			    alert("*** The flowNum already exists in your SMART Deck. \nChoose a different SMART Deck and try again."
				    + "\nFlow name wth same number = " + _userFlowsJSON[i].name + "\nFlow number = " + num);
				 return;
			}
	  }
	  var flowName = flow.name;
	  var flowCat = flow.flowCat;

	  let suggestFlowName = "My_" + flowName;
	  let newFlowName = prompt("Enter your name for " + (justClone?"cloned":"inherited") + " flow name: " + flowName + "\nSuggest: ", suggestFlowName);
	  if (!newFlowName || newFlowName=="")
	  {
	      alert("Cancelled request. Try again later");
			return;
	  }

	  let suggestFlowCat = flowCat + (justClone?"_cloned":"_inherited");
	  let newFlowCat = prompt("Enter your name for " + (justClone?"cloned":"inherited") + " flow cat: " + flowCat + "\nSuggest same: ", suggestFlowCat);
	  if (!newFlowCat || newFlowCat=="")
	  {
	      alert("Cancelled request. Try again later");
			return;
	  }

     let yourUUID = document.getElementById('_uuidStringText').value;
	  if (!confirm((justClone?"Cloning":"Inheriting") + " flow into your SMART Deck with uuid: " + yourUUID + "\nFlowName: " + newFlowName + "\nFlowCat: " + newFlowCat + "\nFlowNum: " + flowNum 
	      + "\n(Note: you must click 'Save Semantic Marker&#x2122;' and confirm overwriting instance"))
	  {
	      alert("Cancelled request. Try again later");
			return;
	  }

	  //todo.. finish.   call setSM()
	  // 1. copy the flow .. and modify the flowCat and flowName and uuid
	  // SOMEONE has to copy the flow ..
	  // if inherit, the flowNum will be the same,
	  // if cloning the flowNum will be unique for this UUID
	  flow.flowNum = flowNum;
	  flow.uuid = yourUUID;
	  flow.name = newFlowName;
	  flow.flowCat = newFlowCat;
	  flow.inheritedSM = _lastCopiedSM.SM;
	  let thisDeckSM = createSM(yourUUID, flowNum);
	  // add to the UUID's flow..
	  _userFlowsJSON.push(flow);
     pushSMLink(thisDeckSM)
	}

}

//! retrieves a SM .. but as it's async .. call back later..
//!  "Retrieve SM"
function retrieveSM(justClone)
{
   // justClone says do all this copying, but use a unique Flow#

   if (!_userFlowsJSON)
	{
      alert("Please click 'Get User SM Flows' to initialize your SMART Buttons");
	   return;
	}
	let SM = prompt("Enter your SM (UUID.flownum):", _lastCopiedSM?_lastCopiedSM.SM:"");
	if (!SM || SM=="")
	{
	  alert("Cancelled request. Try again later");
	  return;
	}

	// check for valid SM
	let parts = SM.split(".");
	if (parts.length != 2)
	{
		alert("Wrong UUID.flownum syntax: " + SM + " - try again");
	   return;
	}

   let uuid = getUUIDFromSM(SM);
	let flowNum = getFlowNumFromSM(SM);
	// store in last copied..
	_lastCopiedSM = {"SM":SM,"uuid":uuid,"flowNum":flowNum, "flow":null};

	//! get the flowName, but it might be null since
	//! async call (if not cached) Call back later..
	let flowName = flowNameOfSM(SM);

	if (flowName)
	{
		// get the flow..
	   _lastCopiedSM.flow = flowOfFlowNum(uuid, flowNum);;
      inheritSM(justClone);
	}
	else
	{
	   alert("Awaiting SM: " + SM + " to be retrieved. Try again soon");
	}
}

//! copySM,  "Copy SM"
function copySM(SM)
{
   if (!_userFlowsJSON)
	{
      alert("Please click 'Get User SM Flows' to initialize your SMART Buttons");
	   return;
	}
  let uuid = getUUIDFromSM(SM);
  let flowNum = getFlowNumFromSM(SM);
  let flow = null;
  for (let i=0; i< _userFlowsJSON.length; i++)
  {
		if (_userFlowsJSON[i].flowNum == flowNum)
		{
			flow = _userFlowsJSON[i];
		   break;
		}
  }
  if (!flow)
  {
     alert("No Flow for SM: " + SM + "\nPlease file bug report");
  }
  else
  {
     var object = {"SM":SM, "uuid": uuid, "flowNum": flowNum, "flow": flow};
     _lastCopiedSM = object;
  }

  // 7.5.23 add to the _savedSMARTDeck
  let smartDeck = document.getElementById('_savedSMARTDeck').value;
  if (smartDeck.length > 0)
     smartDeck += ",";
	smartDeck += SM;
  document.getElementById('_savedSMARTDeck').value = smartDeck;

  // Copy the text inside the text field
  navigator.clipboard.writeText(SM);

  //alert("Copied " + SM);
}

//! creates a flow number (a uuid)
function createFlowNumber()
{
	 var uuid = Date.now();

	 return uuid;
}

//! creates a flow number (a uuid)
function createFlowUUID(flowNumberTextId)
{
	 var uuid = createFlowNumber();

    document.getElementById(flowNumberTextId).value = uuid;
}

//! clears a text area
function clearTextArea(textAreaId)
{
   var currentVal = document.getElementById(textAreaId).value;
	console.log("clearing: " + currentVal);
   document.getElementById(textAreaId).value = "";
}


//! Instructions for adding a field to the FLOW object
/* 
	// maybe a get flow decoded command.
	 3.22.23 

		 KSMatrix - 3a5bab .. 
		 dataSM - location of data information
		 artifactsSM - artifacts
		    NOTE: artifacts could be pointed to from text
		 markup - text or something that might point to the artifacts. Markup language for example
		 languagesSM - info about the language translations..
		 videoSM - link to a video artifact (mp4 ..)
		 isCirularAvatar - whether a circular avatar

	    audioSM -- address of a audio link (or SM)
		    "audioSM
	    locationSMId -- text: id of the string of where this SM was inherited from
		    "locationSM
	    inheritedSMId -- text: id of the string of where this SM was inherited from
		    "inhertedSM
		 X- isDeckId - if a SMART Deck
		    "isDeck
		 deckSMId - a text field of command sepeated addresses
		    "deckSM
		 isMessagingId - if on (set by the owner) then MQTT messagign will ocurr at UUID.flownum topic
		    "whether there are MQTT messages over this Semantic Marker&#x2122; address (UUID.flownum)
       isIndirectSMId - flag if the indirectSm is used (replaces isData)
		    isIndirectSM - flag
		 indirectSMId - if set, then a call will go to this indirect SM 
		    "indirectSM - the address to go to.. the previous isData and nextURL

		 KSWave (address of the Knowledge Shark Wave)
		 bridgeSM (info about the bridge across MQTT Topics
		 future -- placeholder

	inheritedSMId, isDeckId, deckSMId, isMessagingId, isIndirectSMId, indirectSMId))

	msg.flow = {
       "name": msg.payload.name,
       "uuid": msg.payload.uuid,
       "isdata": msg.payload.isdata,
       "private": msg.payload.private,
       "flowNum": msg.payload.flowNum,
       "flowCat": msg.payload.flowCat,
       "date": msg.payload.date,
       "parameters": msg.payload.parameters,
       "desc": msg.payload.desc,
       "QRAvatarURL": msg.payload.QRAvatarURL,
       "nextFlowURL": msg.payload.nextFlowURL,
       "flow": msg.payload.flow,

        //3.22.23
        "audioSM": msg.payload.audioSM,

        "locationSM": msg.payload.locationSM,
        "inheritedSM" : msg.payload.inheritedSM,
        "isDeck": msg.payload.isDeck,
        "deckSM":msg.payload.deckSM,
        "isMessaging":msg.payload.isMessaging,
        "isIndirectSM": msg.payload.isIndirectSM,
        "indirectSM": msg.payload.indirectSM


         //3.25.23
         "videoSM": msg.payload.videoSM,
         "dataSM": msg.payload.dataSM,
         "KSMatrix": msg.payload.KSMatrix,
         "KSWave" : msg.payload.KSWave,
         "bridgeSM": msg.payload.bridgeSM,
         "future": msg.payload.future,
         "artifactsSM":msg.payload.artifactsSM,
         "languagesSM":msg.payload.languagesSM,
         "isCircularAvatar":msg.payload.isCircularAvatar,
         "markup":msg.payload.markup

   }

	//delete node:

	   //!UNfortunately these are manual, so each JSON update, must update these too..
    //!remove these
    delete msg.payload.isdata;
    delete msg.payload.private;
    delete msg.payload.flowNum;
    delete msg.payload.flowCat;
    delete msg.payload.parameters;
    delete msg.payload.desc;
    delete msg.payload.QRAvatarURL;
    delete msg.payload.flow;
    delete msg.payload.flows;
    delete msg.payload.uuid;
    delete msg.payload.name;

    delete msg.payload.nextFlowURL;

    //3.22.23
    delete msg.payload.audioSM;
    delete msg.payload.locationSM;
    delete msg.payload.inheritedSM;
    delete msg.payload.isDeck;
    delete msg.payload.deckSM;
    delete msg.payload.isMessaging;
    delete msg.payload.isIndirectSM;
    delete msg.payload.indirectSM;

    //3.25.23
    delete msg.payload.videoSM;
    delete msg.payload.dataSM;
    delete msg.payload.KSMatrix;
    delete msg.payload.KSWave;
    delete msg.payload.bridgeSM;
    delete msg.payload.future;
    delete msg.payload.artifactsSM;
    delete msg.payload.languagesSM;
    delete msg.payload.isCircularAvatar;
    delete msg.payload.markup;


	#then for the "Grab just the flownum ..

	
	msg.payload.flowJSON2 = JSON.stringify(flow);
    msg.payload.flowJSON = flow;
    msg.payload.uuid = flow.uuid;
    msg.payload.desc = flow.desc;
    msg.payload.flowNum = flow.flowNum;
    msg.payload.flowCat = flow.flowCat;
    msg.payload.flow = JSON.stringify(flow.flow);
    msg.payload.parameters = JSON.stringify(flow.parameters);
    //outer payload.name == username
    // flow.name
    msg.payload.username = msg.payload.name;
    msg.payload.name = flow.name;

    msg.payload.isdata = flow.isdata;
    msg.payload.QRAvatarURL = flow.QRAvatarURL;
    msg.payload.nextFlowURL = flow.nextFlowURL;
    msg.payload.private = flow.private;

    //3.25.23
    msg.payload.videoSM = flow.videoSM;
    msg.payload.dataSM = flow.dataSM;
    msg.payload.KSMatrix = flow.KSMatrix;
    msg.payload.KSWave = flow.KSWave;
    msg.payload.bridgeSM = flow.bridgeSM;
    msg.payload.future = flow.future;
    msg.payload.artifactsSM = flow.artifactsSM;
    msg.payload.languagesSM = flow.languagesSM;
    msg.payload.isCircularAvatar = flow.isCircularAvatar;
    msg.payload.markup = flow.markup;

    //3.22.23
    msg.payload.audioSM = flow.audioSM;

    msg.payload.locationSM = flow.locationSM;
    msg.payload.inheritedSM = flow.inheritedSM;
    msg.payload.isDeck = flow.isDeck;
    msg.payload.deckSM = flow.deckSM;
    msg.payload.isMessaging = flow.isMessaging;
    msg.payload.isIndirectSM = flow.isIndirectSM;
    msg.payload.indirectSM = flow.indirectSM;



	 //in xcode
	 //!3.22.23
    [_orderedFlowKeys addObject:@"locationSM"];
    [_orderedFlowKeys addObject:@"inheritedSM"];
    //[_orderedFlowKeys addObject:@"isDeck" ];
    [_orderedFlowKeys addObject:@"deckSM"];
    [_orderedFlowKeys addObject:@"isMessaging"];
    [_orderedFlowKeys addObject:@"isIndirectSM" ];
    [_orderedFlowKeys addObject:@"indirectSM"];
    [_orderedFlowKeys addObject:@"audioSM"];

	 [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"locationSM"];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"inheritedSM"];
    //[_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"isDeck" ];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"deckSM"];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"isMessaging"];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"isIndirectSM" ];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"indirectSM"];

	 //! 3.25.23
    [_orderedFlowKeys addObject:@"videoSM"];
    [_orderedFlowKeys addObject:@"dataSM" ];
    [_orderedFlowKeys addObject:@"KSMatrix"];
    [_orderedFlowKeys addObject:@"KSWave"];
    [_orderedFlowKeys addObject:@"bridgeSM"];
    [_orderedFlowKeys addObject:@"future" ];
    [_orderedFlowKeys addObject:@"artifactsSM"];
    [_orderedFlowKeys addObject:@"isCircularAvatar"];
    [_orderedFlowKeys addObject:@"markup"];

	 [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"videoSM"];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"dataSM" ];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"KSMatrix"];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"KSWave" ];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"bridgeSM"];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"future"];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"artifactsSM" ];
    [_flowsToTableViewCell setValue:[KSBLEFlowsJSONTableViewCell class] forKey:@"isCircularAvatar"];
    [_flowsToTableViewCell setValue:[KSBLEFlowsTextTableViewCell class] forKey:@"markup"];
*/


//! From: https://jsfiddle.net/user2314737/z437nxaq/
//! update from: https://stackoverflow.com/questions/2109205/open-window-in-javascript-with-html-inserted
//! sends the HTMl element to the printer..
function printDiv(htmlText) 
{
   var handheld = false;
   if (confirm("CANCEL - normal web broser, a new window shows up\n\nOK - if using a handheld device\n"))
	  handheld = true;


   if (!handheld)
   {
	  // first parameters is '' === about:blank
     let mywindow = window.open('', 'SMARTConeTree', 'height=650,width=900,top=100,left=150');
     let title = "SMARTConeTree";

	  //try.. https://stackoverflow.com/questions/1197575/can-scripts-be-inserted-with-innerhtml
	  if (true)
	  {
		  //! TODO.. add this layer of javascript into the generated code (basically what it takes
		  //  to get the inline code to work (like 'playEnded()  and other..., The Slideshow for example)
		  //!!!!! THIS IS WORKING !!!!
  
        let header = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"" +
           "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" +
             "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n";
        var resultConeTree = header + "<title>SMART Cone Tree</title>\n";

		  //! https://html.spec.whatwg.org/multipage/semantics.html
		  //! elements of HTML:
		  //   1. document element (html)
		  //  2. document (head, title, base, link (media, type, link), meta, style)

		  // html 
		  //NO: Gets error: 
		  //Blocked a frame with origin "null" from accessing a cross-origin frame. 
		  // protocols, domains, and ports must match
		  if (false)
		  {
	        var newHtml = mywindow.document.createElement("html");
		     var htmlText = "lang='en'";
	        newHeader.innerHTML = headerText;
	        mywindow.document.html.appendChild(newHtml);
		  }

		  // link header
	     var newLink = mywindow.document.createElement("link");
		  var linkText = "rel='SHORTCUT ICON' href='https://idogwatch.com/iDogWatch.png' type='image/png'";
	     newLink.innerHTML = linkText;
	     mywindow.document.head.appendChild(newLink);

		  //!https://html.spec.whatwg.org/multipage/dom.html#the-document-object
		  // title header
	     var newTitle = mywindow.document.createElement("title");
		  var titleText = "SMART Cone Tree";
	     newTitle.innerHTML = titleText;
	     mywindow.document.head.appendChild(newTitle);

		  //!SM script
	     var newStyle = mywindow.document.createElement("style");
	     var styleText = "@media print{h3 {page-break-before:always}}";
	     newStyle.innerHTML = styleText;
	     mywindow.document.head.appendChild(newStyle);

		  // script header
	     var newScript = mywindow.document.createElement("script");
	     var scriptText = " function playEnded(num) { alert('PlayEnded'); } ";
	     newScript.innerHTML = scriptText;
	     mywindow.document.head.appendChild(newScript);
		  
		  //!SM script
	     newScript = mywindow.document.createElement("script");
	     scriptText = "src='https://SemanticMarker.org/SemanticMarker.js?v=1'"; 
	     newScript.innerHTML = scriptText;
	     mywindow.document.head.appendChild(newScript);

	  }

	  // rest of the text
	  mywindow.document.body.innerHTML = htmlText;
   
     //mywindow.document.close(); // necessary for IE >= 10
     //mywindow.focus(); // necessary for IE >= 10*/
   
   //  mywindow.print();
   //  mywindow.close();
   
   }
   else
   {

	  //keep:
     document.body.innerHTML = htmlText;
   }

  return true;
}

//! 4.26.23 Added start of document printout
//! to stop recursion when  not wanted
//! List of all SM's, from the root out.. 
//! { SM, SMFlow }
var _requestedSMs = [];

//! pass in the ID.. to start with.. This will have a parent=SMART
function traverseSMARTDeck(uuidStringTextId, flowNumId)
{
   if (!_userFlowsJSON)
	{
      alert("Please click 'Get User SM Flows' to initialize your SMART Buttons");
	   return;
	}

  let uuidSelect = document.getElementById(uuidStringTextId);
  let uuid = uuidSelect.value;

  let flowNum = document.getElementById(flowNumId).value;
  if (!flowNum || flowNum.length==0)
  {
      alert("Select a Flow and click 'Show SM Flow'");
	   return;

  }


  //!reset the _base64Images too??
  //NO: lets keep them cached..
  //! 6.27.23  reset the _base65Images too..
  // but let's do it by only deleting those empty..
  //! https://stackoverflow.com/questions/5767325/how-can-i-remove-a-specific-item-from-an-array-in-javascript
  //! use splice
  //!  You can simplify this solution by counting down instead of up:
  for (var i =_base64Images.length - 1; i >= 0; i--)
  {
     // smflow is a JSON object {name, ...}
     let object = _base64Images[i];
     let SM = object.SM;
     let img = object.img;

     if (!img)
     {
        console.log("Removing element:" + i + " SM=" + SM);
        _base64Images.splice(i,1);
     }
  }



  //alert
  alert("Collecting SMART Deck Tree starting at: " + uuid + "." + flowNum
   + "\n\nWait for the number below to go to 0 \nbefore clicking 'Show SMART Cone Tree'"
	+ "\nIf 0 never shows up try to 'Collect' again, or contact support");

    //DARN.. the _requestedSMs is the tree .. so if we don't clean it out .. then the result is not changed.. but appended.. 
	 //TODO.. add a _cachedSMs[] which are the results of the _requestedSMs[] .. unless the cache is out of date .. 
   if (false)
   {
     // check if should clean out the requests..
     var allFinished = allRequestedSMsFinished()
     //if (_requestedSMs.length > 0)
     if (!allFinished)
     {
       if (confirm("Sometimes, web calls fail.\nOK -  clean out the web requests? \nCancel - keep waiting for a request"))
       {
          //reset the array
          _requestedSMs = [];
   
   		 //update the count too
          changeWaitingCount();
        }
     }
   }
   else
   {
          _requestedSMs = [];
   
   		 //update the count too
          changeWaitingCount();
   }
  
  // now recursively traverse starting with this uuid and flowNum
  traverseSMARTDeckFlow(uuid, flowNum, 0, "START");
}

// the resulting cone tree, in order
//    {"SM":thisSM, "smflow":[], "depth": depth, "parentSM":parentSM};
var _coneTree = [];

//! recurse..
/*
call: findChildren(0, "START")

printChildren(depth, parent)
   foreach i at results
      node = results[i]
	   if (node.depth == depth &&
		   node.parent == parent)
			   print(node)
            printChildren(depth+1, node)
		endif
	end
*/

// (1) returns a TEXT of the tree
// (2) creates a _coneTree array with the same order
//    {"SM":thisSM, "smflow":[], "depth": depth, "parentSM":parentSM};
//    
function createConeTreeHTML(depth, parentSM)
{

  //! base case..
  let result = "\n<br>";
  for (let i=0; i< _requestedSMs.length; i++)
  {
    let request = _requestedSMs[i];
    let SM = request.SM;

	 //! at depth requested (the next one down)
	 if (depth == request.depth 
	    && parentSM.localeCompare(request.parentSM)==0)
	 {

		 let uuid = getUUIDFromSM(SM);
		 let flowNum = getFlowNumFromSM(SM);
       let flow = flowOfFlowNum(uuid, flowNum);
    
		 //! get the flowName, but it might be null since
		 //! async call (if not cached) Call back later..
	    let flowName = flow.name; // flowName (but name in JSON0
		 let desc = flow.desc;

		 //! get the markup
		 let markup = flow.markup;
		 let QRAvatarURL = flow.QRAvatarURL;
		 let webAddress = _semanticMarkerFlowURL  + "smart?uuid=" + uuid + "&flow=" + flowNum;

		 result += "<br><h3 id='" + SM + "'>";
		 // print the node (just SM for now)
       for (var x=0; x< depth; x++)
       {
          result += "....";
       }
		 if (flowName)
		    result += flowName;
       result += " (" + SM + ")\n";


		 result += "</h3>\n";
       result += "Links: <a href='#_tableOfContents'>" + "Home" + "</a>\n" ;
       result += "<br><a href='" + webAddress + "'>" + "(Invoke SMART Button - or touch Semantic Marker&#x2122;)" + "</a><br>\n" ;
		 result += "<p>" + desc + "</p><br>\n";

		 //!grab the SM  .. this is from the /getSM web call and is in base64 already
		 for (let z=0;z<_base64Images.length;z++)
		 {

			 let object = _base64Images[z];
		    if (SM.localeCompare(object.SM)==0)
			 {
			    let imageHtml = object.img;
				 //result += "<br>" + imageHtml + "<br>\n";
             result += "<br><a href='" + webAddress + "'>" + imageHtml + "</a><br>\n" ;
			 }

		 }

		 //! draw the QRAvatar
		 result += "<img width='200' src='" + QRAvatarURL + "'</a><br>\n";
		 result += "<br>\n";

		 //! draw the markup
		 //result += markup?markup:"";
		 // if the @(SMARTDeck) is there .. then set this flag
		 let markupHasSMARTDeck = false;
		 if (markup)
		 {
			 // set the flag..
		    markupHasSMARTDeck = markup.includes("@(SMARTDeck)");
			 // finish the markup by replacing links..
		    let finalMarkup = replaceSMLinks(markup, flow, true);
			 result += finalMarkup;
		 }
		 result += "<br>\n";

       // WE can automatically show this .. or for now only if @(SMARTDeck) isn't included, so a web page can be pure
       if (!markupHasSMARTDeck)
       {
    		   //! #### The table for the other avatars
   	      // 4.30.23 show the deckSM links in a pulldown
   	      // let it know what the parent starting this..
            let deckSMLinks = getSMDeckLinksArray(flow.deckSM);
    		   let htmlTable = "<br><b>SMART Deck</b><table>";

         	// use the deckSMLinks array created above .. where the avatarURL is set
         	for (let i=0; i< deckSMLinks.length; i++)
         	{
         	   let object = deckSMLinks[i];
         		let thisSM = object.SM;
         		let flowName = object.flowName;
         		let avatarURL = object.avatarURL;
         	   htmlTable += "<td>" + flowName + "<br><a href='#" + thisSM + "'><img width='100' src='" + avatarURL + "' onClick='showSMLinkSM(\"" + thisSM + "\")'></a></td>\n";
         	}
         
         	// 5.6.23 if the inherited .. tack the button at the end
            let inheritedSM = flowInheritedSMOfSM(SM);
         	if (inheritedSM && inheritedSM.length > 0)
         	{
         	  // tack on inherited at the end..
         	  let flowName = flowNameOfSM(inheritedSM);
         	  let avatarURL = avatarURLOfSM(inheritedSM);

						// grabs the image .. which should already be retrieved
						// this will be base64 image .. with lots of text..
						let semanticMarkerImageHTML = getBase64Image(inheritedSM);
						if (semanticMarkerImageHTML)
						{

						   // the href=#SM is a link inside this document

            	      htmlTable += "<td><table border='2' cellpadding='4' ><tr style='vertical-align:top'><td bgcolor='grey'>Inherited:: " + flowName 
							+ "<br><a href='#" + inheritedSM + "'>" +  semanticMarkerImageHTML + "</a>\n"
            		   + "<br><img width='200' src='" + avatarURL + "' onClick='showSMLinkSM(\"" + inheritedSM + "\")'></a></td>\n";

						}
						else
						{
         	   htmlTable += "<td><table border='2' cellpadding='4' ><tr><td bgcolor='grey'>0.Inherited:: " + flowName 
         		   + "<br><a href='#" + inheritedSM + "'><img width='100' src='" + avatarURL + "' onClick='showSMLinkSM(\"" + inheritedSM + "\")'></a></td>\n";
						}
         	}
         
         
         	htmlTable += "</tr>\n</table><br>\n";
   
   			result += htmlTable;
      		//!NOTE: if the inherited isn't in the table of contents .. then grab it??
       }
 		 //! #### The table for the other avatars

		 //eventually the full USER manual can get generated..
       //! the request is only the the table of contents later..
		 _coneTree.push(request);

		 //now recurse
		 result += createConeTreeHTML(depth+1, SM);
    }
  }
  return result;
}

/*
SMART Deck Cone Tree
QHmwUurxC3.1682633601348 (p=START)
....QHmwUurxC3.100 (p=QHmwUurxC3.1682633601348)
....QHmwUurxC3.1675634815884 (p=QHmwUurxC3.1682633601348)
....QHmwUurxC3.1674080763255 (p=QHmwUurxC3.1682633601348)
........QHmwUurxC3.120 (p=QHmwUurxC3.100)
........QHmwUurxC3.1674080763255 (p=QHmwUurxC3.100)
........QHmwUurxC3.1674080468387 (p=QHmwUurxC3.100)

Requests are IN ORDER (at the same depth)
Replies are in async order
   
SHOULD BE:
0 QHmwUurxC3.1682633601348 (p=START)
1 ....QHmwUurxC3.100 (p=QHmwUurxC3.1682633601348)
2 .......QHmwUurxC3.120 (p=QHmwUurxC3.100)
2 .......QHmwUurxC3.1674080763255 (p=QHmwUurxC3.100) (REPEAT)
2 .......QHmwUurxC3.1674080468387 (p=QHmwUurxC3.100)
1 ....QHmwUurxC3.1675634815884 (p=QHmwUurxC3.1682633601348)
1 ....QHmwUurxC3.1674080763255 (p=QHmwUurxC3.1682633601348)

so at same depth, serach for all depth+1
   and pull under self..
*/

//!show the full deck "Show SMART Cone Tree"
function showFullSMARTDeckTreeHTML()
{
   if (!_userFlowsJSON)
	{
      alert("Please click 'Get User SM Flows' to initialize your SMART Buttons");
	   return;
	}
   if (_requestedSMs.length == 0)
	{
      alert("Please click 'Collect Full SMART Deck Tree' to initialize your SMART Cone Tree");
	   return;
	}

   if (!allRequestedSMsFinished())
	   return;


	//! try to fix the about:blank ???
	//! https://www.freecodecamp.org/news/about-blank-what-does-about-blank-mean-and-why-is-it-blocked-in-chrome-and-firefox/
	//! https://stackoverflow.com/questions/1197575/can-scripts-be-inserted-with-innerhtml

  // empty
  _coneTree = [];
  //!see https://stackoverflow.com/questions/1664049/can-i-force-a-page-break-in-html-printing
   let header = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"" +
        "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" +
        "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n";
  var resultConeTree = header + "<title>SMART Cone Tree</title>\n";
  resultConeTree += "<" + "head><" + "style>@media print{h3 {page-break-before:always}}<" + "/style><" + "/head>\n";
  //try to add some script:
  resultConeTree += "<" + "script>function playEnded(num) { alert('PlayEnded'); } <" + "/script>\n";
  resultConeTree += "<h1>SMART <a href='https://davis.wpi.edu/~matt/courses/trees/'>Cone Tree</a> - generated by Semantic Marker&trade;</h1>\n";

  //!TODO::  this <script> is not getting executed... playEnded() not found..
  //NOTE: the keywords script if shows with <> will break the javascript parser.. I think and think  it's for this code, not the 
  // generated code...

  //! now recurse
  let coneTree = createConeTreeHTML(0,"START");

  // id so others can get back..
  resultConeTree += "<h2 id='_tableOfContents'>Table of Contents</h2>\n";
  //! draw a table of contents..
  for (let i=0; i< _coneTree.length; i++)
  {
		let object = _coneTree[i];
		let name = object.smflow.name;
		let SM = object.SM;
		let depth = object.depth;
		// href links
		for (let x=0;x < depth; x++)
		{
		   resultConeTree += ".....";
		}
      resultConeTree += "<a href='#" + SM + "'>" + name + "</a><br>\n" ;
  }
  resultConeTree += "<br>\n";

  // append the cone tree
  resultConeTree += coneTree;

	  
	  //!TODO:
	  //  create a set of Semantic Markers with some text .. which is
	  // mini manual..

  console.log(resultConeTree);

//  return resultConeTree;
  // now print into an HTML window
  printDiv(resultConeTree);

}

//TODO...
//! traverses the _requestedSMs to see if any 'smflow' still []
function allRequestedSMsFinished()
{
  var allFinished = true;
  //TODO.. collect ths list and show to the user in 1 message
  for (var i =0; i< _requestedSMs.length; i++)
  {
	  // smflow is a JSON object {name, ...}
     let smflow = _requestedSMs[i].smflow;
     let SM = _requestedSMs[i].SM;
	  console.log("request " + SM + "(flow= " + smflow?smflow.name:"*still requested*" + ")");
	  if (!smflow || smflow.length == 0)
	  {
		  alert("Still waiting requestedSM to finish: " + SM);
		  allFinished = false;
		  // exit on first empty..
		  break;
	  }
  }
  for (var i =0; i< _base64Images.length; i++)
  {
	  // smflow is a JSON object {name, ...}
     let object = _base64Images[i];
     let SM = object.SM;
	  let img = object.img;
	  if (!img)
	  {
		  alert("Still waiting SM image to finish: " + SM);
		  allFinished = false;
		  // exit on first empty..
		  break;
	  }
  }
  return allFinished;
}

//!images: https://www.web3.lu/image-manipulations-with-javascript/

//!get just the uuid from the SM (uuid.flownum)
//! return NULL if nothing (wrong syntax)
function getUUIDFromSM(SM)
{
  SM = stripSpaces(SM);
  let SMparts = SM.split(".");
  if (SMparts.length == 2)
  {
    let uuid = SMparts[0];
	 return uuid;
	}
	return null;
}

//!get just the flownum from the SM (uuid.flownum)
//! return NULL if nothing (wrong syntax)
function getFlowNumFromSM(SM)
{
  SM = stripSpaces(SM);
  let SMparts = SM.split(".");
  if (SMparts.length == 2)
  {
    let flowNum = SMparts[1];
	 return flowNum;
	}
	return null;
}

//! add if the SM's UUID isn't there..
//! But the flow will be a singleton.. [flow]
function addToCachedFlow(SM, flowJSON)
{
  // get the uuid
  var uuid = getUUIDFromSM(SM);
  var flowNum = getFlowNumFromSM(SM);

  // look in _cachedFlowsJSON .. if not there then add this single flow
  // if there .. see if flowNum is there .. add to flows
  // 5.1.23
  // push this object, or update the one there..
  var object = {"uuid":uuid,"flows":[flowJSON]};
  var found = false;
  for (let i=0; i< _cachedFlowsJSON.length;i++)
  {
     if (uuid == _cachedFlowsJSON[i].uuid)
	  {
			// look for this flowNum in potentially sparse flows array
			let flowsJSON = _cachedFlowsJSON[i].flows;
			for (let x=0; x < flowsJSON.length; x++)
			{
			   //now look in the flows to see if same flownum
				if (flowsJSON[x].flowNum == flowNum)
				{
				   found = true;
					break;
				}
			}
         // adding a new flowJSON to existing cachedFlowJSON object
         if (!found)
         {
            console.log("pushing cashedFlow(1): " + object.uuid + "." + flowNum);
            _cachedFlowsJSON[i].flows.push(flowJSON);
         }
		}
	}
	if (!found)
	{
      
       console.log("pushing cashedFlow(2): " + object.uuid + "." + flowNum);
		 _cachedFlowsJSON.push(object);
	}
}

//! 5.22.23 try incrementing and decrement the waiting count
function changeWaitingCount()
{
  var count = 0;
  for (var i =0; i< _requestedSMs.length; i++)
  {
	  // smflow is a JSON object {name, ...}
     let smflow = _requestedSMs[i].smflow;
	  if (!smflow || smflow.length == 0)
	  {
	     count++;
	  }
  }
  for (var i =0; i< _base64Images.length; i++)
  {
	  // smflow is a JSON object {name, ...}
     let object = _base64Images[i];
     let SM = object.SM;
	  let img = object.img;
	  if (!img)
	  {
	     count++;
	  }
  }

   console.log("changingWaitingCount: " + count);
   var object = document.getElementById('_waitingCount');
   object.value = count;
   object.style = (count > 0)? "color:red;":"color:green;";
}

//! 4.27.23 start traversing a SMART Deck 
//! 4.29.23 depth is how deep..
//! Recursive 5.7.23 .. add placeholder for Markup
function traverseSMARTDeckFlow(uuid, flowNum, depth, parentSM)
{
  console.log("traverseSMARTDeckFlow(" + uuid + "." + flowNum + " depth=" + depth + " parentSM=" + parentSM + ")");

  let thisSM = createSM(uuid, flowNum);

  // no flow yet..
  let object = {"SM":thisSM, "smflow":[], "depth": depth, "parentSM":parentSM};

  // check if already requested.. if so, then don't request again, but fill it in..
  for (var i =0; i< _requestedSMs.length; i++)
  {
	   let SM = _requestedSMs[i].SM;
	   let smflow = _requestedSMs[i].smflow;
		//console.log("Checking thisSM=" + thisSM + " - against '" + SM + " = " + thisSM.localeCompare(SM));
		if (thisSM.localeCompare(SM) == 0)
		{
			// push .. but reuse the smflow (versus GET web call)
         // NOTE: the requested smflow might still be empty..
			object.smflow = smflow;

			//pushing .. but not requesting again. THis way we see the tree..
         console.log("pushing1 (already requested): " + thisSM  + " depth=" + depth  + " smflow=" + smflow?smflow.name:"still-empty");
         _requestedSMs.push(object);

			// already visited
			return;
		}
	}
   console.log("pushing2 (new request): " + thisSM  + " depth=" + depth  + " smflow=" + object.smflow.length);
   _requestedSMs.push(object);


	//! 5.9.23
	//!new: call the getSM which fills _base64Images JSON array
  requestBase64Image(uuid, flowNum)

  // the address to retrieve. use format=json to get it as well
  let command = _semanticMarkerFlowURL + "smart"
	    + "?uuid=" + uuid + "&flow=" + flowNum  + "&format=json";

  // NOW make an async call .. so the replies are at different times
  // Store the results in the _requestedSMs array
  let xhttp = new XMLHttpRequest();

  // try changing the waiting count..
  changeWaitingCount(); 

  // format=json so this will be json..
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
		//!result is the SM FLOW flows.. [{flow},{flow}]
		let jsonReply = this.responseText;

		//check for error..
		if (jsonReply.indexOf("error") >= 0)
		{
			console.log("ERROR: "+ jsonReply);
         alert("Error retrieving: "+ uuid + "." + flowNum + "\n" + jsonReply);
			return;
		}

		// parse the result look for uuid/flownum and deckSM (the tree)
      let flowJSON = JSON.parse(jsonReply);
		let retrievedSM = createSM(flowJSON.uuid, flowJSON.flowNum);

     // look for the placeholder .. then fill it with the flowJSON
	  for (let i=0; i<_requestedSMs.length; i++)
	  {
		  let SM = _requestedSMs[i].SM;
	     if (SM.localeCompare(retrievedSM)==0)
		  {
			 // potentially add to the cached flows
		    addToCachedFlow(SM, flowJSON);

			 console.log("pushing3: updating instance: " + SM + " smflow : " + flowJSON.name);
		    // update this instance, but only the JSOn smflow {smflow}
			 // flowJSON == fill JSON object {.. name, ...}
			 _requestedSMs[i].smflow = flowJSON;
		  }

	  }

      // try changing the waiting count..
      changeWaitingCount(); 

		// now look for the deckSMs which are the reference to related in this SMART deck 
		// This creates the TREE/GRAPH of the SMART Deck
		let deckSMs = flowJSON.deckSM;
		if (deckSMs && deckSMs.length>0)
		{
		  // now parse the deckSMs, comma seperated
		  deckSMs = stripSpaces(deckSMs);

		  //should be list of UUID.flownum, UUID.flownum
		  let deckSMAddresses = deckSMs.split(",");

		  if (deckSMAddresses && deckSMAddresses.length>0)
		  {
			  // the result if itself if no ','
		     for (let i=0; i< deckSMAddresses.length; i++)
			  {
				  let thisDeckSM = deckSMAddresses[i];
              let deckSMuuid = getUUIDFromSM(thisDeckSM);
              let deckSMflowNum = getFlowNumFromSM(thisDeckSM);

			       // now recurse.. and increment the depth
              traverseSMARTDeckFlow(deckSMuuid, deckSMflowNum, depth+1, thisSM);

					//note the return will be fast, but it's from the async return
			  }
		  }
		}
    }
  };

  // async..
  //get the command, smart?uuid=x&flow=y
  xhttp.open("GET",command, true);
  xhttp.send();

}

//! 5.29.23 Memorial Day, after 2 teams of Draft Horses in Yellow Field Plowing (Sod Busting)

function failed(e) {
   // video playback failed - show a message saying why
   switch (e.target.error.code) {
     case e.target.error.MEDIA_ERR_ABORTED:
       alert('You aborted the video playback.');
       break;
     case e.target.error.MEDIA_ERR_NETWORK:
       alert('A network error caused the video download to fail part-way.');
       break;
     case e.target.error.MEDIA_ERR_DECODE:
       alert('The video playback was aborted due to a corruption problem or because the video used features your browser did not support.');
       break;
     case e.target.error.MEDIA_ERR_SRC_NOT_SUPPORTED:
       alert('The video could not be loaded, either because the server or network failed or because the format is not supported.');
       break;
     default:
       alert('An unknown error occurred.');
       break;
   }
}

//!NOTE: Tricky for the generated cone tree which is HTML .. but doesn't have any
//! of this javascript (such as playEnded).   Can it be added?

// json:  {name, url}
var _musicListURL = [];

// num is the player that finished
function playEnded(num)
{
	//num is player that finished..
	num++;
	if (num >= _musicListURL.length)
	{
      alert("**** Finished Playing Playlist ****");
	}
	else
	{
      var playerName = "_audio_" + num;
      var player = document.getElementById(playerName);
		player.play();
   }
}

//! creates the music player
//! add an edded event: https://stackoverflow.com/questions/5092266/is-there-an-oncomplete-event-for-html5-audio
//! https://html.spec.whatwg.org/#media-resource
//! fire events: https://developer.mozilla.org/en-US/docs/Web/Events/Creating_and_triggering_events
//! https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/addEventListener
//! https://www.w3schools.com/tags/ref_eventattributes.asp
function createMusicPlayer(musicListURLArray)
{

	var htmlString = "";

	for (var i=0; i< musicListURLArray.length; i++)
	{
		var last = (i+1 == musicListURLArray.length);
	   var videoSMJSON = musicListURLArray[i];
		var name = videoSMJSON.name;
		var url = videoSMJSON.url;
		var videoSM = url;
      var mediaName = "audio";
      var mediaType = "audio/mpeg";
		var audioName = "_audio_" + i;

      //htmlString += "<h2>SemanticMarker.org &trade; " + name  + "</h2>\n";
      
      htmlString += "<" + mediaName + " id='" + audioName 
		   + "' width='480' height='320' controls" 
			+ " onended='playEnded(" + i + ")'"
			+ " onerror='failed(event)'>\n";
      htmlString += "<source src='" + videoSM + "' type='" + mediaType + "'>\n";
      
      htmlString += "Sorry, your broswer doesn't support this " + mediaName + ", try this <a href='" + videoSM + "'>link</a>\n";
      htmlString += "</" + mediaName + ">  " + name + "\n";
      htmlString += "<br>";
	}

   return  htmlString;
}

//! creates a slideshow
function createSlideshow(slideshowListURLArray)
{

	var htmlString = "";

	for (var i=0; i< slideshowListURLArray.length; i++)
	{
		var last = (i+1 == slideshowListURLArray.length);
	   var videoSMJSON = slideshowListURLArray[i];
		var name = videoSMJSON.name;
		var url = videoSMJSON.url;

      htmlString += "<img width=50% src='" + url  + "'><br>" + name + "\n";
      
      htmlString += "<br>";
	}

   return  htmlString;
}

var _slideshowListURL = [];

//!parse the field with the playlist..
//! 5.30.23 using artifactsSM for now.
//! return HTML 
function processSlideshow(artifactSM)
{
	// empty
   _slideshowListURL = [];

	var slideshowListURLArray = processToken(artifactSM, "@slideshow");
   var htmlString = createSlideshow(slideshowListURLArray);

	//store globally since the 'next' uses the list..
	_slideshowListURL = slideshowListURLArray;

	return htmlString;
}

//!parse the field with the playlist..
//! 5.29.23 using artifactsSM for now.
//! return HTML 
function processPlaylist(artifactSM)
{
	// empty
   _musicListURL = [];

	var musicListURLArray = processToken(artifactSM, "@playlist");
   var htmlString = createMusicPlayer(musicListURLArray);

	//store globally since the 'next' uses the list..
	_musicListURL = musicListURLArray;

	return htmlString;

}

//!parse the field with the @slideshow(), @playlist() .. as many as desired.
//! 5.30.23 using artifactsSM for now.
//! the resultURL array is the return value
//! token == @playlist or @slideshow
function processToken(artifactSM, token)
{
	var resultURLArray = [];
	// empty
	// -1 if not found
   while (artifactSM.indexOf(token) >= 0)
	{
	  var startToken = artifactSM.indexOf(token);
	  // move pointer
	  artifactSM = artifactSM.substring(startToken);

	  // good to go.. parse this
	  //currently  @playlist("SONG",url,"SONG",url..)
	  //currently  @playlist("title",url,"title",url..)
	  var parenIndex = artifactSM.indexOf("(");
	  parenIndex++;
	  var lastParenIndex = artifactSM.indexOf(")");
	  // start up to the end (but not including the end)
	  var playlistString = artifactSM.substring(parenIndex,lastParenIndex);
	  let playlists = playlistString.split(",");

     //NOTE: Check to be sure the callers' variable for artifactSM isn't modified...
	  //set up the artifactsSM for use again in case there are more @playlist() 
	  // eat up the ')'
	  artifactSM = artifactSM.substring(lastParenIndex+1);

	  if (playlists.length % 2 > 0)
	  {
	     // bad format.. try more..
		  continue;
	  }
	  for (var i=0; i< playlists.length;i=i+2)
	  {

          var name = playlists[i];
			 var URL =  playlists[i+1];
	       //songName.replace(/\"/g, "")

			 var object = {"name":name,"url":URL}; 
          resultURLArray.push(object);
     }
	}
	return resultURLArray;
}

//!! START COOKIE
//! https://www.w3schools.com/js/tryit.asp?filename=tryjs_cookie_username
//! https://developer.mozilla.org/en-US/docs/Web/API/Document/cookie
//! https://www.tutorialspoint.com/javascript/javascript_cookies.htm
//! https://stackoverflow.com/questions/16842226/setting-multiple-cookies-in-javascript
/*
It's counter-intuitive, but setting document.cookie does not overwrite already set cookies, except the one cookie with the same name, if it exists. And the way to remove a cookie, is to set its expiry date to the past. I recommend experimenting in the developer tools of your browser!
*/

//!   This will be name=value
//!   For preferences, this will be 
//! writeCookie appends to document,cookie
//! @see https://www.tabnine.com/academy/javascript/how-to-set-cookies-javascript/  
function writeCookie(name, value)
{
	  //! add expiration
	  var now = new Date();
     now.setMonth( now.getMonth() + 3 );

	  var cookie = name + "=" + encodeURIComponent(value);
     cookie += ";expires=" + now.toUTCString() + "; path=/"

	  console.log("writeCookie: " + cookie);

     document.cookie = cookie;
     //document.cookie = "expires=" + now.toUTCString() + ";"
}

// {"name":name, "value":value}
var _cookieValues = [];

//! reads the cookies, will set the documentID if valid (Note this is called 2 times on startup..)
function readCookies()
{
// 11.21.23 .. no cookies
return;

    var allcookies = document.cookie;

    // Get all the cookies pairs in an array
    cookiearray = allcookies.split(';');

    // Now take key value pair out of this array
    for (var i=0; i<cookiearray.length; i++)
	 {
		   console.log("readCookie: " + cookiearray[i]);
         name = cookiearray[i].split('=')[0];
		   name = name.trim();
         value = decodeURIComponent(cookiearray[i].split('=')[1]);

		   var object = {"name":name,"value":value};
		   _cookieValues.push(object);

		  if (name.localeCompare("_username")==0)
		  {
	        var val = object.value;
			  _username = val;
		  }

		  else if (name.localeCompare("_password")==0)
		  {
	        var val = object.value;
			  _password = val;;
		  }
		  else
		  {
		    // the hidden cookies 

         let isHidden = value;
	   	if (isHidden.localeCompare("true")==0)
			 {
		       // start with 1 cookie "SMARTButton = true/false)
		       if (name.localeCompare("_SMRecordDevice")==0)
		       {
                hideShowMarkup(['_SMRecordDevice','_m88.99', '_m88.1','_m88.2','_m88.3','_m88.4','_m88.5','_m88.6','_markupTextID','markupText']);
				 }
		       // start with 1 cookie "SMARTButton = true/false)
		       else if (name.localeCompare("_m5Commands")==0)
		       {
                hideShowMarkup(['_m5Commands']);
				 }
				 //add:
				 else
				 {
          //      hideShowMarkup([name]);
				 }
			 }
			}

     }
}

//! writes the cookies
//! This reads the _parameters which are elementId's
function saveCookies()
{

// 11.21.23 no cookies
	 // THis will take the _username and _password and save them..
    setSemanticMarkerUserPasswordValues();

    return;

	 var cookieNames = ['_SMRecordDevice','_m5Commands','_password','_username', '_GroupCommands'];

	 for (var i = 0; i < cookieNames.length; i++)
	 {
	    var cookieName = cookieNames[i];


		 // OR?? can the _username of the cookiename be used?
		 //var val = cookieNames[i];
	    // lets start with the SMART Button IoT set..
		  if (cookieNames[i].localeCompare("_username")==0)
		  {
	        var object = {"name":cookieName,"value":_username};
	        writeCookie(object.name, object.value);
		  }
		  else if (cookieNames[i].localeCompare("_password")==0)
		  {
	        var object = {"name":cookieName,"value":_password};
	        writeCookie(object.name, object.value);
		  }
		  else
		  {
		    //hidden fields
	        // lets start with the SMART Button IoT set..
           var isHidden = document.getElementById(cookieName).hidden;
    
	         var object = {"name":cookieName,"value":isHidden};
				console.log("saveCookie: " + JSON.stringify(object));
	         writeCookie(object.name, object.value);
		  }

	 }
	 // THis will take the _username and _password and save them..
    setSemanticMarkerUserPasswordValues();
}


//!! END COOKIE



//! 6.12.23 reload by calling lookupuser with our known user-name and password
function reloadUserAgent()
{

   // _rootBotURL = "https://iDogWatch.com/bot";

  // the address to retrieve. use format=json to get it as well
  let command = _rootBotURL + "/lookupuser/"
	    +  _username + "/" + _password;

  console.log("reloadUserAgent: " + command);

  // NOW make an async call .. so the replies are at different times
  // Store the results in the _requestedSMs array
  let xhttp = new XMLHttpRequest();

  // format=json so this will be json..
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
		//!result is the SM FLOW flows.. [{flow},{flow}]
		let jsonReply = this.responseText;

		//check for error..
		var errors = ["error","Invalid","Wrong password"];
		for (let i =0; i< errors.length;i++)
		{
		   if (jsonReply.indexOf(errors[i]) >= 0)
		   {
			   console.log("ERROR: "+ jsonReply);
            if (confirm("Error retrieving: " + _username + "\n\nTry again?"))
				{
					logout();
				}
				else
				{
					_username = "";
				   initWebPage();
	            document.getElementById("_Login").hidden = false;
				}
			   return;
			}
		}

		console.log("REPLY = " + jsonReply);
		 
		// parse into JSON
      var userJSON = JSON.parse(jsonReply);

		console.log("userJSON = " + JSON.stringify(userJSON));

		//set the guest password 
		_guestPassword = userJSON.guestPassword;

		//! 10.31.23 add the Guest Passwords
		_guestPasswordsJSON = userJSON.guestPasswords;
		if (!_guestPasswordsJSON)
		   _guestPasswordsJSON = [];

		console.log("GuestPasswords = " + JSON.stringify(_guestPasswordsJSON));
		createGuestPasswordListOptions('_guestPasswordsSelectID');

		// set the global for the devices right now
		_devicesJSON = userJSON.devices;

		// it's the devices that can change..
		// Update the Options of devices and devicesToPair
      createDeviceListOptions('devices');
      createDeviceListOptions('_devicesToPair');

		//! 1.22.24 added 
      createDeviceListOptions('_devicesToRenameTo'); 
// NEW .. add to smart.html version *********
		//! NEW: 
		_groupsJSON = userJSON.groups;

		if (!_groupsJSON)
		   _groupsJSON = {"groups":[]};

		//! all the groups
      createGroupsListOptions("_groups");
   
		//! all the groups and their devices names (what you call them, not the group name)
      createGroupDeviceListOptions("_groups", "_groupDevices");

		//12.12.23 (Amber in Paris) .. set the semantic markers (username,password,guestpassword)
      setSemanticMarkerUserPasswordValues()

// **** THIS IS A PLACE TO update when ther..
if (!_useOldSMARTDeck)
{
      getSMSmartDecks(null);
}
else
{
		// grab smartDecks
		_smartDecks = userJSON.smartDecks;

		//! 10.3.23 can be NULL or undefined
		if (!_smartDecks)
		{
		   alert("First Time UUID Creation. No SMART Decks yet.");
		   _smartDecks = [];
		}

		var uuid = userJSON.uuid;

	   // update the deck options  (Call from getSMUserUUID...
      parseSmartDecks(uuid);
}

    }
  };

  // async..
  xhttp.open("GET",command, true);
  xhttp.send();
}

//!https://stackoverflow.com/questions/901115/how-can-i-get-query-string-values-in-javascript`
function getParameterByName(name, url = window.location.href) {
    name = name.replace(/[\[\]]/g, '\\$&');
    var regex = new RegExp('[?&]' + name + '(=([^&#]*)|&|#|$)'),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, ' '));
}

//!check query parms for username password
//!https://stackoverflow.com/questions/901115/how-can-i-get-query-string-values-in-javascript`
// support:  ?username=NAME&password=P
function checkQueryParms()
{
   var username = getParameterByName('username'); // "lorem"
   var password = getParameterByName('password'); // "" (present with empty value)
   var guestpassword = getParameterByName('guestpassword'); // "" (present with empty value)
	if (username)
	{
	   _username = username;
	   _password = password;
		_guestPassword = guestpassword;
      saveCookies();
	}
}

/* **************************** GROUPS ************************ */
// the groups JSON that will be retrieved from the agents "group" field
var _groupsJSON = 

    [{"group":"atlasDogs",
         "devices":[{ "dev":"ScoobyDoo","display":"Scott.ScoobyDoo", "color":"green"} ,
                     { "dev":"PumpkinUno",    "display": "Scott.PumpkinUno",  "color":"orange"},
                     { "dev":"Wilma",    "display": "Kristina.Wilma",  "color":"red"}
                    ] },
      {"group":"houndDogs", "devices": [] }
    ];

//!create the list of devices.. Fill in the idDeviceListOptions (which has the select already formed)
function createGroupsListOptions(idGroupListOptions)
{

  var htmlSelect = "";
  htmlSelect += "<option value='NO_GROUP'>NO_GROUP</option>";
  htmlSelect += "\n";

  // used for sorting later..
  var groupNames = [];

  var groupsArray = _groupsJSON;
  console.log(groupsArray);

  // go through list, but dont include GENERIC and DEFAULT
  for (var i =0; i< groupsArray.length; i++)
  {
		  // key = group name

			  var groupJSON = groupsArray[i];
		     var groupName = groupJSON.group;

		     // how to get the name
		     //Not sorting yet..
          //if ("_GENERIC_".localeCompare(deviceName)!=0 && "_DEFAULT_".localeCompare(deviceName) != 0)
           {
			    groupNames.push(groupName);
		     }
	}

	//now sort
	groupNames.sort();
   for (var i =0; i< groupNames.length; i++)
   {
     let deviceName = groupNames[i];

	  // create option for the deviceName
     htmlSelect += "<option value='" + deviceName + "'>" + deviceName + "</option>";
     htmlSelect += "\n";
   }

  document.getElementById(idGroupListOptions).innerHTML = htmlSelect;
}

//!create the list of devices.. Fill in the idDeviceListOptions (which has the select already formed)
function createGroupDeviceListOptions(idGroupListOptions, idGroupDeviceListOptions)
{

  // The current selected group name
  // Only show devices from this group
  var selectedGroupName = document.getElementById(idGroupListOptions).value;

  var htmlSelect = "";
  htmlSelect += "<option value='--ALL--'>--ALL--</option>";
  htmlSelect += "\n";

  // used for sorting later..
  var groupDeviceNames = [];

  var groupsArray = _groupsJSON;
  console.log(groupsArray);

  // go through list, but dont include GENERIC and DEFAULT
  for (var i =0; i< groupsArray.length; i++)
  {
		  // key = group name

			  var groupJSON = groupsArray[i];
		     var groupName = groupJSON.group;
			  var devicesArray = groupJSON.devices;

			  if (groupName != selectedGroupName)
			     continue;

			  for (var k=0; k< devicesArray.length; k++)
			  {
			     var device = devicesArray[k];
				  //  dev, display, color
              //"devices":[{ "dev":"ScoobyDoo","display":"Scott.ScoobyDoo", "color":"green"} ,
				  var dev = device.dev;
				  var display = device.display;
				  var color = device.color;

	           // create option for the deviceName
              htmlSelect += "<option value='" + dev + "'>" + display + "</option>";
              htmlSelect += "\n";

			  }
	}

  document.getElementById(idGroupDeviceListOptions).innerHTML = htmlSelect;
}

//! wraps the command no args
function sendGroupMessage(idGroups, idGroupDevices, idCmd)
{
	// the command. NOTE: there are some that use the "send" syntax still 5.4.23
	var cmd = document.getElementById(idCmd).value;
	var group = document.getElementById(idGroups).value;
	var groupDevice = document.getElementById(idGroupDevices).value;

	// if group == "NO_GROUP" then do nothing..
	if (group == "NO_GROUP")
	{
      alert("Please specify a group to send message to");
		return;
	}

	var deviceSpecified = true;
	if (groupDevice == "--ALL--")
	{
	   deviceSpecified = false;
	}

   var command = _rootBotURL + "/";
	var userInfo = _username + "/" + _password;


	if (cmd == "status")
	   command += "statusgroup/" + userInfo + "/" + group;
	else if (cmd == "capture")
	   command += "capturegroup/" + userInfo + "/" + group;
	else if (cmd == "feed")
	{
      if (deviceSpecified)
	      command += "feeddevicegroup/" + userInfo + "/" + groupDevice + "/" + group;
		else
	      command += "feedgroup/" + userInfo + "/" + group;
	}

  console.log(command);

  // NOW make an async call .. so the replies are at different times
  sendGETCommand(command);
}

//! wraps the command no args
function sendAddNewUser(idUser, idUserPassword, idGuestPassword, idToken)
{
   sendURL("adduser", [idUser, idUserPassword, idGuestPassword, idToken]);
}

//! wraps changeuserpassword
function sendChangeYourPassword(idChangeYourPasswordUserName,idChangeYourPassword,idChangeYourNewPassword)
{
   sendURL("changeuserpassword", [idChangeYourPasswordUserName,idChangeYourPassword,idChangeYourNewPassword]);
}

//! aggregator of the send URL call
//! valueArray already have the values 
function sendURLValues( commandName, valueArray)
{
   
	// add user to group topic
   var command = _rootBotURL + "/" + commandName;

	// go through the idArray
	for (var i =0; i< valueArray.length;i++)
	{
      var parm = valueArray[i];

		command += "/" + parm;

	}
   console.log(command);

   if (confirm(command))
   {
     // NOW make an async call .. so the replies are at different times
      callGetCommandReload(command);
   }

}

//! aggregator of the send URL call
//! idArray is array of id's to document.getElementById()
function sendURL( commandName, idArray)
{

   var valArray = []
	// go through the idArray
	for (var i =0; i< idArray.length;i++)
	{
      var id = idArray[i];
	   var parm = stripSpaces(document.getElementById(id).value);
		valArray.push(parm);
	}
	sendURLValues(commandName, valArray);

}

//! wraps updateuser
function sendUpdateUser(idUpdateUserName, idUpdateUserPassword, idUpdateUserGuestPassword, idTokenUpdateUser)
{
   sendURL("updateuser", [idUpdateUserName, idUpdateUserPassword, idUpdateUserGuestPassword, idTokenUpdateUser]);
}

//! wraps changeguestpassword
function sendChangeGuestPassword(idChangeGuestPasswordUserName,idChangeGuestPassword,idChangeGuestPasswordGuestPassword)
{
   sendURL("changeguestpassword", [idChangeGuestPasswordUserName,idChangeGuestPassword,idChangeGuestPasswordGuestPassword]);
}

//! wraps the command no args
function sendAddUserToGroup(idGroups, idUser, idToken)
{
	// the command. NOTE: there are some that use the "send" syntax still 5.4.23
	var group = stripSpaces(document.getElementById(idGroups).value);

	// if group == "NO_GROUP" then do nothing..
	if (group == "NO_GROUP")
	{
      alert("Please specify a group to send message to");
		return;
	}

   //BUG: wrong order 1.9.24 
	// sendURL("addusertogrouptopic", [idGroups, idUser, idToken]);
   sendURL("addusertogrouptopic", [idUser, idGroups, idToken]);
}


//! add new group to the agents JSON
function addNewGroup(idNewGroupName)
{
   var groupName = stripSpaces(document.getElementById(idNewGroupName).value);

	var values = [_username, _password, groupName];
	sendURLValues("addgroup",values);
}

//! remove group to the agents JSON
function removeGroup(idGroupName)
{
   var groupName = stripSpaces(document.getElementById(idGroupName).value);
	if (groupName == "NO_GROUP")
	{
	  alert("A Group name must be specified");
	  return;
	}

	var values = [_username, _password, groupName];
	sendURLValues("removegroup",values);

}
 
//! add new device to specified group to the agents JSON (if ALL then don't do it..)
function addNewGroupDevice(idGroups, idNewGroupDeviceName, idNewGroupDisplayDeviceName)
{
   var groupName = stripSpaces(document.getElementById(idGroups).value);
   var groupDeviceName = stripSpaces(document.getElementById(idNewGroupDeviceName).value);
   var groupDisplayDeviceName = stripSpaces(document.getElementById(idNewGroupDeviceName).value);
	var color = "unused";

	//note: this string compare works.. forget that localeCompare
	if (groupName == "NO_GROUP")
	{
	  alert("A Group name must be specified");
     return;
	}

	if (groupName.length == 0 || groupDeviceName.length == 0 || groupDisplayDeviceName.length ==0)
	{

	  alert("Group Device names cannot be empty");
	  return;
	}

   var values = [_username, _password, groupDeviceName, groupDisplayDeviceName, color, groupName];
	sendURLValues("adddevicetogroup", values);
}

//! 7.27.23 .. heading to hike Tolmie Peak on a nice day
function subscribeGroups(idDevices, idSubscribeGroupNames)
{
   // use the 'set' syntax
	// names can have command and #  (is # allowed on url??)
	var groupNames = stripSpaces(document.getElementById(idSubscribeGroupNames).value);

	// parse for valid "," etc..
	var groupNamesString = "";
	// same as C:  strchr(groupNames,'.') != NULL)
	const groupNamesArray = groupNames.split(',');
	for (var i=0;i<groupNamesArray.length;i++)
	{
	   if (i > 0)
	      groupNamesString += ", ";
		   
	   groupNamesString += groupNamesArray[i];
	}
	
	if (!confirm("Subscribing to groups: " + groupNamesString))
	{
	   return;
	}
   


   groupNames = groupNames.replace(/[#]/g, "%23");

	var anySetCommand = "groups";
	var anySetVal = groupNames;


   //!  set:cmd, val:arg
   //! THese use the values passed in for 2nd and 3rd parm
   sendAnySetCommand2(idDevices, anySetCommand, anySetVal)

}

//! 1.13.24 for group names in all messages
var _useGroupNameInMessages = false;
//! get the current group name. NULL if not set or NONE
//! query sytax returns "" or "?group=NAME" or "&group=NAME"
//! if the argument appendQuery is true, use the "&group=name"
function getCurrentGroupNameQuery(appendQuery)
{
	var queryArg = "";
	if (!_useGroupNameInMessages)
	   return queryArg;

   var groupName = document.getElementById('_groups').value;
	var validGroup = true;
	if (!groupName || groupName.length == 0)
	{
      validGroup = false;
	}
	else if (groupName == "NO_GROUP")
	   validGroup = false;

	if (validGroup)
	{
	   if (appendQuery)
		   queryArg = "&";
		else
		   queryArg = "?";
		queryArg = queryArg + "group=" + groupName;
	}
	return queryArg;
}
//! sets the global value _useGroupNameInMessages
//! idButtonBoolFlag is "on" or "off"  -- so convert to boolean
function setUseGroupNamesFlag(idBoolFlag, idButtonBoolFlag)
{
  var flag = document.getElementById(idBoolFlag).value;
  //flag == "on' "off"  not true/falue
  var booleanFlag = false;
  if (flag == "on")
     booleanFlag = true;
  else if (flagg == "true")
     booleanFlag = true;

  _useGroupNameInMessages = booleanFlag;
  console.log("*** Setting _useGroupNameInMessages = " + booleanFlag);

  //! now change the color of the button (red if using the group)
  var buttonId = document.getElementById(idButtonBoolFlag);
  if (booleanFlag)
     buttonId.className = "M5button";
  else
     buttonId.className = "button";

}
//! end 1.13.24 group names

//TODO.. get the RUN SemanticMarkers working...
//! make web get call
function callGetCommandReload(url)
{
   var xhttp = new XMLHttpRequest();

	//!https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readystatechange_event
   xhttp.onreadystatechange = function() {
		// called when open is finished
      if (this.readyState == 4 && this.status == 200)
	   {
		    let jsonReply = this.responseText;
		    if (jsonReply)
			 {
			    if (jsonReply.indexOf("Wrong") >= 0
				 ||
			        jsonReply.indexOf("Invalid") >= 0
				 )
			    {
			       alert(jsonReply);
			    }
			 }
      }
		//reload the agent
      reloadUserAgent();
   };
   //! shows the command at this element
   document.getElementById('_messageId').innerHTML = url;

   xhttp.open("GET", url, true);
   xhttp.send();
}

/* **************************** END GROUPS ************************ */

//   ***************************** start timer ***************************

// ***** Interval Timer ****
//https://www.w3schools.com/js/js_timing.asp
var _countdownTimer;
var _timerRunning = false;
// these defaults need to be same as text defaults (since isn't initialized unless clicking on items)
var _intervalSeconds = 5;
var _intervalSecondsMax = 5;
var _timerDurationMinutes = 1; // minutes
// start time .. so if the 
var _startTimerTime;
// end time .. based on the duration
var _endTimerTime;
var _countdownTime;

/*
Globals: getElementById()
   'timerButton'
   'timeLeftButton'
   'timeInButton'
	'_numFeedsButton'
Parameters:
	durationMinutesId
   minSecondsId
   maxSecondsId
*/

//! calculate the duration
function setupTimerDuration()
{

	// milliseconds .. 60,000 milliseconds/minute
   _startTimerTime = (new Date()).getTime();

	_endTimerTime = _startTimerTime + (_timerDurationMinutes* 60000);

	document.getElementById('_numFeedsButton').value = 0;

}

//! returns if over the duration
function finishTimerDuration()
{
	// milliseconds
   var timeNowMilliseconds = (new Date()).getTime();
	var isOver =  (timeNowMilliseconds > _endTimerTime);

	var milliSec =  _endTimerTime - timeNowMilliseconds;
	var sec = Math.floor((milliSec/1000) % 60);
   var min = Math.floor(((milliSec/1000) / 60) % 60);
   var hour =Math.floor( ((milliSec/1000) / 60) / 60);

	if (isOver)
	   sec = min = hour = 0;

	var timeLeftString = hour + ":" + min + ":" + sec +  " (hr:min:sec)";
   document.getElementById('timeLeftButton').value = timeLeftString;


	return isOver;

}

//! sets the timer label
function setTimerLabel(isStart)
{
	var startString = isStart?"Start Timer":"Stop Timer";
   document.getElementById('_timerButton').value = startString;

}
//! random between two numbers min/max
//! see https://www.w3schools.com/js/js_random.asp
function getRandomInteger(min, max) 
{
  min = parseInt(min);
  max = parseInt(max);

  let randomInt=  Math.floor(Math.random() * (max - min) + min)
  console.log("random(" + min + "," + max + ") = " + randomInt);
  if (randomInt < min)
  {
     alert("Num < min = " + randomInt);
  }

  return randomInt;
}

//Sets the timer
function setIntervalTimerMinMax(minSecondsId, maxSecondsId, durationMinutesId)
{
   var seconds = parseInt(document.getElementById(minSecondsId).value); 
   var secondsMax = parseInt(document.getElementById(maxSecondsId).value); 

	// make the max at least the min
	if (seconds >= secondsMax)
	{
	   secondsMax = seconds;
      document.getElementById(maxSecondsId).value = String(secondsMax); 
	}

	// find the duration minutes
	var durationMinutes = document.getElementById(durationMinutesId).value; 


   _intervalSeconds = seconds;
   _intervalSecondsMax = secondsMax;
	_timerDurationMinutes = durationMinutes;

   // not changing state..
   var _countdownTime = getRandomInteger(_intervalSeconds, _intervalSecondsMax);
   if (_timerRunning)
   {
      clearInterval(_countdownTimer);
      _timerRunning = false;
   }
   document.getElementById('timeInButton').value = _countdownTime;
	setTimerLabel(true);
//   document.getElementById('_timerButton').style.color = "black";

}

//start stop button touched
function startStopTimer()
{

   var time = _intervalSeconds;
   if (_timerRunning == false)
   {
		// start the timer
//      document.getElementById('_timerButton').style.color = "red";
	   setTimerLabel(false);
      _timerRunning = true;
      // init the timer
      initTimer();
		// only setup the timer on first start..
      setupTimerDuration();

		// update the time label
      finishTimerDuration();
   }
   else
   {
		// sstop the  timer
	   setTimerLabel(true);
//      document.getElementById('_timerButton').style.color = "black";

      // clear/stop the timer
      clearInterval(_countdownTimer);
      _timerRunning = false;
   }
}
//every second .. to update countdown
function initTimer()
{
   var seconds = getRandomInteger(_intervalSeconds, _intervalSecondsMax);

   _countdownTime = seconds;

	//! call the timer: with seconds
	//!   setInterval(function, millisecods)
	//! https://www.w3schools.com/jsref/met_win_setinterval.asp
   _countdownTimer = setInterval(updateTimer, 1000) // set up a setInterval for the countdown function

	// call every second
  finishTimerDuration();

}


// https://stackoverflow.com/questions/9609731/onchange-event-for-input-type-number
// called each second, so change label, etc
function updateTimer()
{
  var isDone = finishTimerDuration();
  if (isDone)
  {
	  startStopTimer();
     alert("timer duration complete: " + _timerDurationMinutes + " minutes");
	  return;
  }

  // change the timeInButton label
  document.getElementById('timeInButton').value = _countdownTime;
  if (_countdownTime <= 0 )
  {
      //restart
      //callFeedCommand();
		//! same code as "Send" up above..
      sendNoArgs('devices','commandNoArg');

		//TODO .. generic FEED command
		var command = document.getElementById('commandNoArg').value;
      document.getElementById('timeInButton').value = command;
      clearInterval(_countdownTimer);
      initTimer();

	   var numFeeds = document.getElementById('_numFeedsButton').value;
		numFeeds++;
	   document.getElementById('_numFeedsButton').value = numFeeds;
      return;
  }
  else
  {
      //document.getElementById("demo").value = " ";
   }
  _countdownTime--;

}

//   ***************************** end timer ***************************

// creates a help button, looking at all the HelpClass items
function createHelpButtons()
{

  var allHelpClassArray = document.getElementsByClassName("HelpClass");
  for (var i=0; i< allHelpClassArray.length; i++)
  {
      var helpObject = allHelpClassArray[i];

//TODO.. 
  }

}

//! load with the selectOptionId
//! NOTE: accessing 'device' scope tricky: 
//! https://www.javatpoint.com/javascript-onload
window.onload = function()
{

   checkQueryParms();

	//! 6.13.23 if not supplied then request user login
	//requestLoginWindow(true);

	// 9.5.23 change
	requestLoginWindow(false);

	//if valid .. then perform the rest of this initialization
   initWebPage();

//	createHelpButtons();

}

// the rest of the init code .. this can be called with empty values..
function initWebPage()
{
	// all that want to register
   initRegisteredDevices();

	// update the 2 places where devices are shown
   createDeviceListOptions('devices');
   createDeviceListOptions('_devicesToPair');
		//! 1.22.24 added 
   createDeviceListOptions('_devicesToRenameTo'); 

   var htmlText = "User Page for <a href='https://idogwatch.com/bot/userpage/" + _username + "/" + _password + "'>" + _username + "</a>";
   htmlText += " ";
   //htmlText += "<a href='https://idogwatch.com/bot/userpage2/" + _username + "/" + _password + "'>" + " (NewPage) " + "</a>";
   htmlText += "<a href='https://idogwatch.com/userpage/userpage.html?username=" + _username + "&password=" + _password + "'>" + " (NewPage) " + "</a>";
   document.getElementById('userId').innerHTML = htmlText;

	// grab the UUID as well
   getSMUserUUID('_uuidStringText');

	// update the deck options  (Call from getSMUserUUID...
   //parseSmartDecks();

	 // cookies stores the state of the hidden..
	 // This deals with _SMRecordDevice  and the 
	 readCookies();

	 //start with hidden fields..
    hideShowMarkup(['m2.1','m3.1','m10.1','m1Note','m2Note']);

    //! 3.3.24 also hide the SMART Button
	 hideShowMarkup(['_SMRecordDevice','_m88.99', '_m88.1','_m88.2','_m88.3','_m88.4','_m88.5','_m88.6','_markupTextID','markupText','sm1','descriptionText']);
	 // 3.3.24 more.. of the SMART Button .. merge later
    hideShowMarkup([ '_SMButton.1', '_SMButton.2', '_SMButton.3', '_SMARTMarkupTableID']);

	 // 3.3.24 hide the matrix
    hideShowMarkup(['mSM_Matrix.2','mSM_Matrix.3', 'mSM_Matrix.4','mSM_Matrix.5', 'mSM_Matrix.8']);

    // 3.3.234 the Semantic Flows
	 hideShowMarkup(['mSM.2','sm1','_sm11','_sm12','_SMFlow10']);

    // the OTA
    hideShowMarkup(['mOTA.1','_RevertToMainOTA','mOTA.2','_QAOTA','mOTA.3','_DevOTA']);
    hideShowMarkup(['_m99.1']);

    hideShowMarkup(['_UserAccounts']);

    hideShowMarkup(['_GroupCommands']);

    hideShowMarkup(['_m5Commands','_m5Commands.2','_btnContainer1','_btnContainer2']);

	 // The smart buttons
    hideShowMarkup(['_scotty']);
}

//! just sets the values in the setAny64, the user still has to click the button themselves
//.   idSetOption (select option), idValOption (select option), idSetCommand (where set goes), idValString (where val goes)
function setCommand64Option(idSetOption, idValOption, idSetCommand, idValString)
{
   var setOption = document.getElementById(idSetOption).value;
   var valOption = document.getElementById(idValOption).value;

   document.getElementById(idSetCommand).value = setOption;
   document.getElementById(idValString).value = valOption;
//   alert("Set options. User must still click 'setCommand64' when finished");
}
//! **************** 3.3.24 New SM Matrix Support *********************

// this is false if SM Matrix shown
var _showOnlyAvatar = false;
var _matrixJSON = 

{"matrix":[
 [
  {"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1698208438983",
	 "uuid":"V6gZLkRley",
	 "device":"MaggieMae",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://knowledgeshark.me/docs/IoT_Pitch_AW_Nov21/IoT_Pitch_AW_Nov21.059.jpeg",
     "paragraph":"1.1finalSemanticMarkerImage",
     "pdevice":"1.1device"
   },

{"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1",
	 "uuid":"uuid",
	 "device":"ScoobyDoo",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://user-images.githubusercontent.com/5956266/218890699-09eb9fc2-01c9-465d-a017-6f912831942d.jpeg",
     "paragraph":"1.2finalSemanticMarkerImage",
     "pdevice":"1.2device"
   },
{"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1",
	 "uuid":"uuid",
	 "device":"ScoobyDoo",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://user-images.githubusercontent.com/5956266/219557658-7e40a085-0079-4852-bf6a-8dfb02731e27.jpeg",
     "paragraph":"1.3finalSemanticMarkerImage",
     "pdevice":"1.3device"
   }
],

[ {"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1",
	 "uuid":"uuid",
	 "device":"ScoobyDoo",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://SemanticMarker.org/images/M5/M5Menu.jpg",
     "paragraph":"2.1finalSemanticMarkerImage",
     "pdevice":"2.1device"
   },

{"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1",
	 "uuid":"uuid",
	 "device":"ScoobyDoo",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://KnowledgeShark.me/images/SMMatrixSMARTImage.png",
     "paragraph":"2.2finalSemanticMarkerImage",
     "pdevice":"2.2device"
   },
{"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1",
	 "uuid":"uuid",
	 "device":"ScoobyDoo",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://SemanticMarker.org/users/QHmwUurxC3/Decks/rooms/Room1.png",
     "paragraph":"2.3finalSemanticMarkerImage",
     "pdevice":"2.3device"
   }
],
 [{"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1",
	 "uuid":"uuid",
	 "device":"ScoobyDoo",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://SemanticMarker.org/users/QHmwUurxC3/Decks/rooms/Room2.png",
     "paragraph":"3.1finalSemanticMarkerImage",
     "pdevice":"3.1device"
   },
{"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1",
	 "uuid":"uuid",
	 "device":"ScoobyDoo",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://user-images.githubusercontent.com/5956266/218890699-09eb9fc2-01c9-465d-a017-6f912831942d.jpeg",
     "paragraph":"3.2finalSemanticMarkerImage",
     "pdevice":"3.2device"
   },
{"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1",
	 "uuid":"uuid",
	 "device":"ScoobyDoo",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://user-images.githubusercontent.com/5956266/221418254-a93d02bf-31b4-46fa-b740-bfde510c43aa.jpeg",
     "paragraph":"3.3finalSemanticMarkerImage",
     "pdevice":"3.3device"
   }
]
]
}
;


//! go through the matrix and perform the operations
function createSMMatrix(showOnlyAvatar)
{
  // save state
  _showOnlyAvatar = showOnlyAvatar;
  var matrix = _matrixJSON["matrix"];
  for (var row =0; row <  matrix.length; row++)
  {
      var rowJSON = matrix[row];
		for (col =0; col < rowJSON.length; col++)
		{
			var itemJSON = rowJSON[col];
		   console.log(JSON.stringify(itemJSON));
			document.getElementById('semanticMarkerAvatarURL').value = itemJSON.QRAvatarURL;
			document.getElementById('semanticMarkerURL').value = itemJSON.URL;
			var paragraphElement = document.getElementById(itemJSON.paragraph);
			var pdeviceElement = document.getElementById(itemJSON.pdevice);

			var device = itemJSON.device;

			//@see https://imagekit.io/blog/how-to-resize-image-in-html/
			if (showOnlyAvatar)
			{
			    paragraphElement.innerHTML = '<img style="max-height:300px; max-width:300px;" src="' + itemJSON.QRAvatarURL + '">';
			}
			else
			{
            createSemanticMarkerPathId('semanticMarkerURL','semanticMarkerAvatarURL',itemJSON.paragraph,'semanticMarkerURLId');
			}
			pdeviceElement.innerHTML = device;

		}
	}
}

//TODO.. get the RUN SemanticMarkers working...
//! make web get call
function callGetCommand(url)
{
   var xhttp = new XMLHttpRequest();

	//!https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readystatechange_event
   xhttp.onreadystatechange = function() {
		// called when open is finished
      if (this.readyState == 4 && this.status == 200)
	   {
		    let jsonReply = this.responseText;
		    if (jsonReply)
			 {
			    if (jsonReply.indexOf("Wrong") >= 0
				 ||
			        jsonReply.indexOf("Invalid") >= 0
				 )
			    {
			       alert(jsonReply);
			    }
			 }
      }
		//reload the agent
      //reloadUserAgent();
   };
   //! shows the command at this element
   //document.getElementById('_messageId').innerHTML = url;

   xhttp.open("GET", url, true);
   xhttp.send();
}
//! just web invokes the SemanticMarker
function invokeSemanticMarkerWebAddress(webAddress)
{
	//var webAddress = document.getElementById(smId).value;

	//if (window.confirm("Invoke: " + webAddress + "?"))
	{
	   // invoke this manually..
      //window.open(webAddress, 'SemanticMarkerWindow');
      callGetCommand(webAddress);
	}
}
//! called to invoke the selected Semantic Marker
//! row/col == 1 based
function invokeSMMatrixRowCol(row, col)
{

	var matrixArray = _matrixJSON.matrix;
	var matrixItem = matrixArray[row-1][col-1];

	var uuid = matrixItem.uuid;
	var flowNum = matrixItem.flowNum;
	var device = matrixItem.device;

	// value to use
   //TODO: the smrun isn't implemented yet .. except for #FEED  (need to fix this..)
   var useSMRun = (document.getElementById("_useSMRun").value == "1");
	var SMURL = "https://SemanticMarker.org/bot/";
	if (useSMRun)
	   SMURL += "smrun";
	else
	   SMURL += "smart";
	SMURL += "?uuid=" + uuid + "&flow=" + flowNum;
	

	SMURL += "&username=" + _username;
	SMURL += "&password=" + _password;
	SMURL += "&device=" + device;
   invokeSemanticMarkerWebAddress(SMURL);

   if (useSMRun)
      callGetCommand(SMURL);
	else
	   window.open(SMURL, 'SemanticMarkerWindow');
	
	//TODO: make this without bringing up the web page..

}

//! called to invoke the selected Semantic Marker
function invokeSMMatrixCell(matrixRowNumId, matrixColNumId)
{
   var row = document.getElementById(matrixRowNumId).value;
   var col = document.getElementById(matrixColNumId).value;

	if (row =="" || col=="")
	{
	   alert("Please select the row/col, currently: " + row + "," + col);
		return;

	}

	invokeSMMatrixRowCol(row, col);
}
//! 3.3.24 Almost Snowing. Poweder in hills.
//! This will updeate the _matrixJSON row/col with appropriate info..
/*
var _matrixJSON = 

{"matrix":[
 [
  {"SM":"V6gZLkRley.1698208438983",
    "Name":"My_3_LivingRoom",
    "Cat": "NDAChallenges_inherited",
	 "flowNum":"1",
	 "uuid":"uuid",
	 "device":"device",
     "InheritedSM": "QebkVLtEvPDI.1698208438983",
     "URL": "https://SemanticMarker.org/bot/smart?uuid=V6gZLkRley&flow=1698208438983&flowname=My_3_LivingRoom&flowcat=NDAChallenges_inherited",
     "QRAvatarURL":"https://knowledgeshark.me/docs/IoT_Pitch_AW_Nov21/IoT_Pitch_AW_Nov21.059.jpeg",
     "paragraph":"1.1finalSemanticMarkerImage"
     "pdevice":"1.1device"
   },
*/
function saveSMInRowCol(flowsSelectId, matrixRowNumId, matrixColNumId, idDevices)
{
	// saves _deviceName;
	// sets _isSendToAll (which we cannot use ...
   setupDevice(idDevices);

   var row = document.getElementById(matrixRowNumId).value;
   var col = document.getElementById(matrixColNumId).value;

	if (row =="" || col=="")
	{
	   alert("Please select the row/col, currently: " + row + "," + col);
		return;

	}

	var matrixArray = _matrixJSON.matrix;
	var matrixItem = matrixArray[row-1][col-1];
	

  var selectedFlowIndex = document.getElementById(flowsSelectId).value;
  var flow = _userFlowsJSON[selectedFlowIndex];
  var flowName = flow.name;
  var private = flow.private;
  var isdata = flow.isdata;
  var uuid = flow.uuid;
  var flowNum = flow.flowNum;
  var flowCat = flow.flowCat;
  var desc = flow.desc;
  var date = flow.date;
  var QRAvatarURL = flow.QRAvatarURL;
  var nextFlowURL = flow.nextFlowURL;


   matrixItem.Name = flowName;
   matrixItem.Cat = flowCat;
	matrixItem.SM = uuid + "." + flowNum;
	matrixItem.QRAvatarURL = QRAvatarURL;
	matrixItem.uuid = uuid;
	matrixItem.flowNum = flowNum;
	matrixItem.device = _deviceName;

	matrixItem.URL = "https://SemanticMarker.org/bot/smart?uuid=" + uuid + "&flow=" + flowNum;

	//! fill in the matrix showing the user what's been selected
	var tableTextCellId = row + "." + col + "SM";
	var tableTextCell = document.getElementById(tableTextCellId);
	tableTextCell.innerHTML = flowCat + "." + flowName + "(" + _deviceName + ")";

	//! 3.5.24 now redisplay matrix
   createSMMatrix(_showOnlyAvatar);

}

//! create a user version of the matrix that can be pasted back later
//! syntax:  row,col,cat,name, uuid,flownum \n
function showSMMatrixDefinition(textAreaId)
{

	var result = "";
	var matrixArray = _matrixJSON.matrix;
	for (var row=1; row < 4; row++)
	{
	  for (var col=1; col< 4; col++)
	  {
	     var matrixItem = matrixArray[row-1][col-1];
		  //! syntax:  (row,col,cat,name, uuid,flownum)

        var flowName = matrixItem.Name;
        var flowCat = matrixItem.Cat;
	     var uuid = matrixItem.uuid;
	     var flowNum = matrixItem.flowNum;
	     var SM = matrixItem.SM;
		  var device = matrixItem.device;

		  var rowString = row + "," + col + "," + flowCat + "," + flowName + "," + uuid + "," + flowNum  + "," + device;

		  result += rowString + "\n";
	  }
	}

	document.getElementById(textAreaId).value = result;

}

//! create a user version of the matrix that can be pasted back later
//! syntax:  row,col,cat,name, uuid,flownum, device \n
function importSMMatrixDefinition(textAreaId)
{
   var maxCols = 7;

	result = document.getElementById(textAreaId).value;
//	result = stripSpaces(result);
// seems to get rid of New lines too..

	var rows = result.split("\n");
	for (var rowIndex =0; rowIndex < rows.length; rowIndex++)
	{
		var thisRow = rows[rowIndex];
		thisRow = stripSpaces(thisRow);

		if (thisRow.length == 0)
		   continue;

	   var rowItems = thisRow.split(",");
		if (rowItems.length != maxCols)
		{
		   alert("Syntax Error, you need to enter as specified. You entered: " + thisRow);
			return;
		}
		var row = rowItems[0];
		var col = rowItems[1];
		var cat = rowItems[2];
		var name = rowItems[3];
		var uuid = rowItems[4];
		var flowNum = rowItems[5];
		var device = rowItems[6];

      //! see if this SM.uuid is our UUID, if so then return the flowName
      //! returns the flow 
		var flow = flowOfFlowNum( uuid, flowNum);
	   if (flow)
		{
         //alert(JSON.stringify(flow)); 
	      var matrixArray = _matrixJSON.matrix;
	      var matrixItem = matrixArray[row-1][col-1];
	

         var flowName = flow.name;
         var private = flow.private;
         var isdata = flow.isdata;
         var uuid = flow.uuid;
         var flowNum = flow.flowNum;
         var flowCat = flow.flowCat;
         var desc = flow.desc;
         var date = flow.date;
         var QRAvatarURL = flow.QRAvatarURL;
         var nextFlowURL = flow.nextFlowURL;
       
       
         matrixItem.Name = flowName;
         matrixItem.Cat = flowCat;
       	matrixItem.SM = uuid + "." + flowNum;
       	matrixItem.QRAvatarURL = QRAvatarURL;
       	matrixItem.uuid = uuid;
       	matrixItem.flowNum = flowNum;

       	matrixItem.device = device;

	      matrixItem.URL = "https://SemanticMarker.org/bot/smart?uuid=" + uuid + "&flow=" + flowNum;

	      //! fill in the matrix showing the user what's been selected
	      var tableTextCellId = row + "." + col + "SM";
	      var tableTextCell = document.getElementById(tableTextCellId);
	      tableTextCell.innerHTML = flowCat + "." + flowName + "(" + device + ")";
		}
		else 
		{
         alert("Flow isn't loaded: " + uuid + "," + flowNum);
			return;
		}
	}

	//! 3.5.24 now redisplay matrix
   createSMMatrix(_showOnlyAvatar);

}

#endif
