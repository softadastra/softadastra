<?php
namespace Modules\Map\Core\Http\Controllers;

use App\Controllers\Controller;
use Ivi\Http\HtmlResponse;

class HomeController extends Controller
{
    public function index(): HtmlResponse
    {
        // Titre de la page
        $title = (string) (cfg(strtolower('Map') . '.title', 'Softadastra Map') ?: 'Softadastra Map');
        $this->setPageTitle($title);

        // Message pour la vue
        $message = "Hello from MapController!";

        // 🔹 Correct: module_asset avec Core et tag HTML généré automatiquement
        $styles  = module_asset('Map/Core', 'assets/css/style.css');
        $scripts = module_asset('Map/Core', 'assets/js/script.js');

        return $this->view(strtolower('Map') . '::home', [
            'title'   => $title,
            'message' => $message,
            'styles'  => $styles,
            'scripts' => $scripts,
        ]);
    }
}