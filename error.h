#pragma once


#define EOK 0
#define EOOM 1
#define ENDX 2 // Index out of range

#define E_END ENDX


#define trye(f, err, label) {(err) = f; if ((err) != EOK) goto label;}
#define try(f, label) trye(f, err, label)
#define seterre(code, err, label) {(err) = code; goto label;}
#define seterr(code, label) seterre(code, err, label)
