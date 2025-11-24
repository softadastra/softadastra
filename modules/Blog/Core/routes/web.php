<?php
use Modules\Blog\Core\Http\Controllers\HomeController;
use Ivi\Http\JsonResponse;

/** @var \Ivi\Core\Router\Router $router */
$router->get('/blog', [HomeController::class, 'index']);
$router->get('/blog/ping', fn() => new JsonResponse([
    'ok' => true,
    'module' => 'Blog/Core'
]));