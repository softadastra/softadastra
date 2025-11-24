<?php
use Modules\Map\Core\Http\Controllers\HomeController;
use Ivi\Http\JsonResponse;

/** @var \Ivi\Core\Router\Router $router */
$router->get('/map', [HomeController::class, 'index']);
$router->get('/map/ping', fn() => new JsonResponse([
    'ok' => true,
    'module' => 'Map/Core'
]));