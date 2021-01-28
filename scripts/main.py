# language_override:  "en"
from engine import TR1ItemId, I18n

from scripts.tr1.object_infos import object_infos
from scripts.tr1.audio import tracks
from scripts.tr1.level_sequence import level_sequence, title_menu, lara_home

cheats = {
    "godMode": True,
    "inventory": {
        TR1ItemId.Key1: 10,
        TR1ItemId.Key2: 10,
        TR1ItemId.Key3: 10,
        TR1ItemId.Key4: 10,
        TR1ItemId.Puzzle1: 10,
        TR1ItemId.Puzzle2: 10,
        TR1ItemId.Puzzle3: 10,
        TR1ItemId.Puzzle4: 10,
    }
}


def getGlidosPack():
    return None
    # return "assets/trx/1SilverlokAllVers/silverlok/"
    # return "assets/trx/JC levels 1-12/Textures/JC/"
    # return "assets/trx/JC levels 13-15/Textures/JC/"


def getObjectInfo(id):
    return object_infos[TR1ItemId(id)]


def getTrackInfo(id):
    return tracks[id]


i18n = {
    I18n.LoadGame: {
        "en": "Load Game",
        "de": "Spiel laden",
    },
    I18n.SaveGame: {
        "en": "Save Game",
        "de": "Spiel speichern",
    },
    I18n.NewGame: {
        "en": "New Game",
        "de": "Neues Spiel",
    },
    I18n.ExitGame: {
        "en": "Exit Game",
        "de": "Spiel beenden",
    },
    I18n.ExitToTitle: {
        "en": "Exit to Title",
        "de": "Zur~uck zum Titel",
    },
    I18n.EmptySlot: {
        "en": "- EMPTY SLOT {}",
        "de": "- LEERER SLOT {}",
    },
    I18n.Items: {
        "en": "ITEMS",
        "de": "GEGENST~ANDE",
    },
    I18n.GameOver: {
        "en": "GAME OVER",
        "de": "SPIEL VORBEI",
    },
    I18n.Option: {
        "en": "OPTION",
        "de": "EINSTELLUNG",
    },
    I18n.Inventory: {
        "en": "INVENTORY",
        "de": "INVENTAR",
    },
    I18n.Game: {
        "en": "Game",
        "de": "Spiel",
    },
    I18n.Controls: {
        "en": "Controls",
        "de": "Steuerung",
    },
    I18n.Sound: {
        "en": "Sound",
        "de": "Audio",
    },
    I18n.DetailLevels: {
        "en": "Detail Levels",
        "de": "Grafik",
    },
    I18n.LarasHome: {
        "en": "Lara's home",
        "de": "Laras Zuhause",
    },
    I18n.Compass: {
        "en": "Compass",
        "de": "Kompass",
    },
    I18n.Pistols: {
        "en": "Pistols",
        "de": "Pistolen",
    },
    I18n.Shotgun: {
        "en": "Shotgun",
        "de": "Schrotflinte",
    },
    I18n.Magnums: {
        "en": "Magnums",
        "de": "Magnums",
    },
    I18n.Uzis: {
        "en": "Uzis",
        "de": "Uzis",
    },
    I18n.Grenade: {
        "en": "Grenade",
        "de": "Granate",
    },
    I18n.LargeMediPack: {
        "en": "Large Medi Pack",
        "de": "Gro=es Medi Pack",
    },
    I18n.SmallMediPack: {
        "en": "Small Medi Pack",
        "de": "Kleines Medi Pack",
    },
}

print("Yay! Main script loaded.")
