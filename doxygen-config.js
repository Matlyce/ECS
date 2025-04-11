// Initialisation des composants doxygen-awesome-css

// Attendre que le DOM soit complètement chargé
document.addEventListener("DOMContentLoaded", function () {
    // Activer le bouton de basculement du thème sombre
    if (window.DoxygenAwesomeDarkModeToggle) {
        window.DoxygenAwesomeDarkModeToggle.init();
    }

    // Activer les boutons de copie des fragments de code
    if (window.DoxygenAwesomeFragmentCopyButton) {
        window.DoxygenAwesomeFragmentCopyButton.init();
    }

    // Activer les liens de paragraphes
    if (window.DoxygenAwesomeParagraphLink) {
        window.DoxygenAwesomeParagraphLink.init();
    }

    // Activer la table des matières interactive
    if (window.DoxygenAwesomeInteractiveToc) {
        window.DoxygenAwesomeInteractiveToc.init();
    }
});