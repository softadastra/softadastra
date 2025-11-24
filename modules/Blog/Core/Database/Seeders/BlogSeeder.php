<?php
declare(strict_types=1);

namespace Modules\Blog\Core\Database\Seeders;

final class BlogSeeder
{
    public function run(): void
    {
        echo "[seed] Blog ok\n";
    }
}

return new BlogSeeder();