<?php
namespace Modules\Blog\Core\Http\Controllers;

use App\Controllers\Controller;
use Ivi\Http\HtmlResponse;

class HomeController extends Controller
{
    public function index(): HtmlResponse
    {
        // Titre de la page
        $title = (string) (cfg(strtolower('Blog') . '.title', 'Softadastra Blog') ?: 'Softadastra Blog');
        $this->setPageTitle($title);

        // Message pour la vue
        $message = "Hello from BlogController!";

        // 🔹 Correct: module_asset avec Core et tag HTML généré automatiquement
        $styles  = module_asset('Blog/Core', 'assets/css/style.css');
        $scripts = module_asset('Blog/Core', 'assets/js/script.js');

        return $this->view(strtolower('Blog') . '::home', [
            'title'   => $title,
            'message' => $message,
            'styles'  => $styles,
            'scripts' => $scripts,
        ]);
    }
}