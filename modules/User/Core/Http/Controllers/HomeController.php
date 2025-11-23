<?php

namespace Modules\User\Core\Http\Controllers;

use App\Controllers\Controller;
use Ivi\Http\HtmlResponse;
use Modules\Auth\Core\Helpers\AuthRedirect;

class HomeController extends Controller
{
    public function index(): HtmlResponse
    {
        AuthRedirect::redirectIfGuest('/auth/login');

        // 🔹 Assets
        $styles  = module_asset('User/Core', 'assets/css/style.css');

        return $this->view(strtolower('User') . '::home', [
            'title'    => 'My Account',
            'styles'   => $styles,
        ]);
    }
}
