------------------------------------------------
-- 3 letter race identifiers to race id numbers
------------------------------------------------

--player races
HUM = 1
HUF = 1
BAM = 2
BAF = 2
ERM = 3
ERF = 3
ELM = 4
ELF = 4
HIM = 5
HIF = 5
DAM = 6
DAF = 6
HAM = 7
HAF = 7
DWM = 8
DWF = 8
TRM = 9
TRF = 9
OGM = 10
OGF = 10
HOM = 11
HOF = 11
GNM = 12
GNF = 12
IKM = 128
IKF = 128
--vah? not sure what file they're in

--npc races
AVI = 13 --aviak
WER = 14 --werewolf - assumed
BRM = 15 --brownie
BRF = 15
CEN = 16 --centaur
GOL = 17 --golem
GIA = 18 --giant
TRK = 19 --trakanon
VST = 20 --ventril sathor
BEH = 21 --beholder/evil eye
BET = 22 --beetle
CPM = 23 --kerra (?!)
CPF = 23
FIS = 24 --fish
FAM = 25 --fairy - not sure why these are gendered, guess they were lazy about using 'M' and 'F' to indicate gender
FAF = 25
FRO = 26 --froglok soldier
FRG = 27 --froglok
FUN = 28 --fungus man
GAR = 29 --gargoyle
IGR = 29 --gargoyle from sleeper_chr
GAM = 29 --gargoyle from velketor_chr

CUB = 31 --gelatinous cube
GHM = 32 --ghost?
GHU = 33 --ghoul
BAT = 34 --bat

RAT = 36 --rat
SNA = 37 --snake
SPI = 38 --spider
GNN = 39 --gnoll
GOB = 40 --goblin
GOR = 41 --gorilla
WOM = 42 --wolf
WOF = 42 --scaled wolf
WOL = 42 --also wolf, I guess
BEA = 43 --bear
FPM = 44 --freeport citizen
FPF = 44
DML = 45 --demi-lich / sathir's girlfriend
IMP = 46 --imp
GRI = 47 --griffin
KOB = 48 --kobold
DRA = 49 --dragon
LIM = 50 --lion
LIF = 50
LIZ = 51 --lizard man
MIM = 52 --mimic / living chest
MIN = 53 --minotaur
ORC = 54 --orc
BGM = 55 --human beggar
BGF = 55
PIM = 56 --pixie
PIF = 56
DRM = 57 --draknid
DRF = 57
SOL = 58 --old solusek ro
BGG = 59 --bloodgill goblin
SKE = 60 --classic skeleton / luclin skeleton
SHA = 61 --shark
TUN = 62 --tunare
TIG = 63 --tiger
TRE = 64 --treant
DVM = 65 --maestro of rancor
DVF = 65
RAL = 66 --old rallos zek
HHM = 67 --highkeep citizen
HHF = 67
TEN = 68 --tentacle terror
TEM = 68
WIL = 69 --will-o-wisp
ZOM = 70 --zombie / mummy
ZOF = 70
QCM = 71 --qeynos citizen
QCF = 71

PIR = 74 --pirahna
ELE = 75 --classic elemental / luclin elemental
PUM = 76 --puma
NGM = 77 --neriak citizen
NGF = 77
EGM = 78 --erudin citizen
EGF = 78
BIX = 79 --bixie
REA = 80 --animated hand
RIM = 81 --rivervale citizen
RIF = 81
SCA = 82 --scarecrow
SKU = 83 --skunk
SNE = 84 --snake elemental ?
SPE = 85 --spectre
SPH = 86 --sphinx
ARM = 87 --armadillo
CLM = 88 --clockwork gnome / akanon citizen
CLF = 88
DRK = 89 --drake
HLM = 90 --halas citizen
HLF = 90
ALL = 91 --alligator
GRM = 92 --grobb citizen
GRF = 92
OKM = 93 --oggok citizen
OKF = 93
KAM = 94 --kaladim citizen
KAF = 94
CAZ = 95 --cazic thule
COC = 96 --cockatrice
DIA = 97 --daisy man ?
VSM = 98 --mistmoore vampire
VSF = 98
DEN = 99 --denizen / amygdalan
DER = 100 --dervish
EFR = 101 --efreeti
FRT = 102 --froglok tadpole
KED = 103 --kedge
LEE = 104 --leech
SWO = 105 --swordfish
FEM = 106 --felwithe citizen
FEF = 106
MAM = 107 --mammoth / elephant
EYE = 108 --eye of zomm
WAS = 109 --wasp
MER = 110 --mermaid
HAR = 111 --harpy
GFM = 112 --kelethin citizen
GFF = 112
DRI = 113 --drixie / chromatic drake
GSP = 114 --ghost ship ?
CLA = 115 --clam ?
SEA = 116 --seahorse
GDM = 117 --dwarf ghost
GDF = 117
GEM = 118 --erudite ghost
GEF = 118
STC = 119 --sabretooth tiger
WOE = 120 --wolf ghost / spirit wolf
GRG = 121 --gorgon
DRU = 122 --undead dragon / dracoliche
INN = 123 --innoruuk
UNI = 124 --unicorn / horse
PEG = 125 --pegasus
DJI = 126 --djinni
IVM = 127 --invisible man
--iksar
SCR = 129 --scorpion
--vah shir
SRW = 131 --sarnak
DRK = 132 --draglok ? there are a lot of ids for nonexistent models
LYC = 133 --drolvarg
MOS = 134 --mosquito
RHI = 135 --rhinoceros
XAL = 136 --xalgoz
KGO = 137 --kunark goblin
YET = 138 --yeti
ICM = 139 --cabilis citizen
ICF = 139
FGI = 140 --kunark giant
--boat 141
--minor illusion 142
--tree illusion 143
BRN = 144 --burynai
GOO = 145 --goo
SSN = 146 --spectral sarnak
SIM = 147 --spectral iksar
SIF = 147
BAC = 148 --kunark fish
ISC = 149 --iksar scorpion
ISK = 149
ERO = 150 --erollisi marr (fitting)
TRI = 151 --tribunal
BER = 152 --bertoxxulous
BRI = 153 --bristlebane
FDR = 154 --fay drake, 32nd president of the united states
SSK = 155 --sarnak skeleton
VRM = 156 --rat man
WYV = 157 --wyvern
WUR = 158 --wurm
DEV = 159 --devourer
IKG = 160 --iksar golem
IKS = 161 --iksar skeleton
MEP = 162 --man-eating plant
RAP = 163 --raptor
SGO = 164 --sarnark golem
SED = 165 --faydedar / water dragon
IKH = 166 --iksar animated hand
SUC = 167 --succulent
FMO = 168 --flying monkey / holgresh
BTM = 169 --brontothermium / velious rhino
SDE = 170 --snow dervish
DIW = 171 --dire wolf
MTC = 172 --manticore
TOT = 173 --totem man
SPC = 174 --ice spectre
ENA = 175 --enchanted armor
SBU = 176 --snow bunny
WAL = 177 --walrus
RGM = 178 --geonid / rock gem man

YAK = 181 --yakkar / yak man
FAN = 182 --faun ?
COM = 183 --coldain / thurgadin citizen
COF = 183
CON = 183 --dain frostreaver
DR2 = 184 --velious dragon
HAG = 185 --hag
HIP = 186 --hippogriff ?
SIR = 187 --siren
FSG = 188 --frost giant
STG = 189 --frost giant guard
STM = 189
OTM = 190 --othmir / otter man
WLM = 191 --walrus man
CCD = 192 --clockwork dragon
ABH = 193 --abhorrent
STU = 194 --sea turtle
BWD = 195 --black / white dragon
GDR = 196 --ghost dragon
--RONNIE TEST, the perfect model that never existed
PRI = 198 --prismatic dragon, better known as Kerafrym the Sleeper
--luclin models start here
SKN = 199 --shik'nar
RHP = 200 --rockhopper
UNB = 201 --underbulk
GMM = 202 --grimling
GMF = 202
GMN = 202
VAC = 203 --vacuum worm
--EVAN TEST
KHA = 205 --khati shah / shadel
OWB = 206 --owlbear
RNB = 207 --rhino beetle
VPM = 208 --luclin vampire
EEL = 209 --earth elemental
AEL = 210 --air elemental
WEL = 211 --water elemental
FEL = 212 --fire elemental
