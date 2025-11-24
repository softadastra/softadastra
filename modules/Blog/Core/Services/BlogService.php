<?php
declare(strict_types=1);

namespace Modules\Blog\Core\Services;

final class BlogService
{
    public function info(): string
    {
        return 'Module Blog loaded successfully.';
    }
}