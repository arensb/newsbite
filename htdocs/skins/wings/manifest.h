CACHE MANIFEST
/* Must begin with the "CACHE MANIFEST" line.*/
/* IMPORTANT: Must increment the number (or make some change) to
 * trigger a reload
 */
## Manifest version NONCE

/* All of the following (relative) URLs will be cached locally: */
style.css
wings.js
images/gray-bg.jpg
#ifdef MOBILE
/* iPhone/iPad desktop icon */
images/icon.png
#  if defined(IPHONE)
images/splash.png
#  elif defined(IPAD)
images/splash-ipad.png
images/splash-ipad-landscape.png
#  else
/* Default splash image */
images/splash.png
#  endif	// iPhone/iPad
#endif	// MOBILE
/favicon.ico

/*
# (mandatory) URLs that match the following prefixes will be whitelisted:
#NETWORK:
#http://www.ooblick.com/app/
*/
/*
# More cache entries after network entries:
CACHE:
*/
/*
# Fallback section. Each line contains two URLs. If the first one is
# inaccessible, use the second one instead.
FALLBACK:
#/files/projects        /projects

# What Safari on fromage thinks this page contains:
#CACHE:
#http://www.ooblick.com/newsbite/?skin=wings
#skins/wings/style.css
#skins/wings/wings.js
#skins/wings/images/gray-bg.jpg
#http://www.ooblick.com/favicon.ico
#feeds.php
*/
