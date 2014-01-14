Brightcove integration API and field. Features:

* Browse videos and playlists coming from Brightcove Studio directly in Drupal with a possibility to search in them by text.
* Upload a videos and playlists for Brightcove customers with at least a Professional account (You need a Write API key).
* Integrates with Views module - creates fields with a video player, all available metadata such as Plays count or Creation dates.
* Provides various formatters


Requirements
------------

* Entity API
  http://drupal.org/project/entity

* Ctools
  http://drupal.org/project/ctools

* External libraries: Brightcove PHP MAPI Wrapper
  http://opensource.brightcove.com/project/PHP-MAPI-Wrapper/

Installation instructions
-------------------------

Download and enable Entity API module (version >= 1.3).

Download and enable Ctools.

Download Brightcove module (http://drupal.org/project/brightcove), untar to sites/all/modules or sites/sitename.com/modules

Download Brightcove PHP MAPI Wrapper from http://opensource.brightcove.com/project/PHP-MAPI-Wrapper/, module is tested with PHP MAPI Wrapper 2.0.4 and later.

Unzip framework to sites/all/libraries/*. After unzipping, there needs to be a file at sites/all/libraries/*/bc-mapi.php.

Enable Brightcove module and Brightcove Field module at your site.

Get Read and/or Write API keys from Brightcove support. Refer to section Brightcove Keys if you don't know how to do that.

Navigate to admin/config/media/brightcove and fill in your Read/Write key and default Player ID from Brightcove Studio (Refer to section Brightcove player if you don't know how to do that)

Navigate to admin/config/media/brightcove/players and create a default player for the videos/playlists.

Create a new field called of type Brightcove field and select the proper widget type (Playlist/Video).

Play.

Brightcove PlayerKEY
-----------------------------------------

On admin settings page (admin/config/media/brightcove) you must enter the Brightcove PlayerKEY. Here's the process how to find it:
1 - Sign in to the Brightcove Studio,
2 - Go to the Publishing module and select the player,
3 - Click Get Code to copy the player publishing code to your clipboard,
4 - Find, copy and paste the playerKey value to your own embed code.


