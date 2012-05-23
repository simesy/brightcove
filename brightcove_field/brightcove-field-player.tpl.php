<?php

/**
 * @file brightcove-field-player.tpl.php
 * Default template for embeding brightcove players.
 *
 * Available variables:
 * - $id
 * - $width
 * - $height
 * - $classes_array
 * - $bgcolor
 * - $flashvars
 *
 * @see template_preprocess_brightcove_field_embed().
 */
?>

<object id="<?php print $id;?>" width="<?php print $width;?>" class="<?php print join($classes_array, ',');?>"
     height="<?php print $height;?>" classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
     codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=9,0,47,0">
   <param name="movie" value="http://c.brightcove.com/services/viewer/federated_f9?isVid=1&isUI=1" />
   <param name="bgcolor" value="<?php print $bgcolor;?>" />
   <param name="base" value="http://admin.brightcove.com" />
   <param name="seamlesstabbing" value="false" />
   <param name="allowFullScreen" value="true" />
   <param name="swLiveConnect" value="true" />
   <param name="allowScriptAccess" value="always" />
   <param name="flashVars" value="<?php print $flashvars;?>" />

   <embed src="http://c.brightcove.com/services/viewer/federated_f9?isVid=1&isUI=1"
     bgcolor="<?php print $bgcolor;?>"
     flashVars="<?php print $flashvars;?>"
     name='flashObj'
     wmode="transparent"
     base="http://admin.brightcove.com"
     width="<?php print $width;?>"
     height="<?php print $height;?>"
     seamlesstabbing="false"
     type="application/x-shockwave-flash"
     allowFullScreen="true"
     swLiveConnect="true"
     allowScriptAccess="always"
     pluginspage="http://www.macromedia.com/shockwave/download/index.cgi?P1_Prod_Version=ShockwaveFlash">
   </embed>

</object>
