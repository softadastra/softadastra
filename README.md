# 🚀 Softadastra Drive — Core Backend (C++ / Vix.cpp)

Softadastra Drive is the **foundation of the new Softadastra platform** —  
a **cloud-first, offline-capable, real-time, AI‑augmented storage engine** designed for developers and modern distributed applications.

This repository hosts the **high-performance C++ backend**, powered by **Vix.cpp**, built for:

- Ultra-fast file operations
- Modern sync mechanisms
- Durability across devices
- Local-first behaviour with cloud consistency
- Integration with a dedicated AI engine (`drive-ai`)

Softadastra Drive is the first brick of the future **Softadastra Global OS + WorldNet**, a unified environment that enables:

- Cloud desktop
- App studio
- Marketplace
- AI-native workflows
- Peer-to-peer sync

Drive‑core is the **heart** that everything else depends on.

---

# 🌍 Vision

Softadastra Drive aims to redefine what a cloud storage engine can be:

### **💾 1. Storage that feels local**

Files load fast, sync instantly, and remain available **offline**, thanks to IndexedDB and a local-first sync algorithm.

### **🔄 2. Real-time sync like Apple iCloud — but open**

Every update triggers instant events:

- `file_created`
- `file_updated`
- `file_deleted`
- `folder_changed`

All exposed via WebSocket for any client, SDK, or app.

### **🧠 3. AI-Native storage**

Every file can be:

- summarized,
- OCR‑processed,
- semantically indexed,
- explored via natural language.

A PDF becomes searchable.  
An image becomes tagged.  
A folder becomes intelligent.

### **🧩 4. Open SDK ecosystem**

Developers can integrate Softadastra Drive via:

- **JavaScript SDK**
- **C++ SDK**
- **Python SDK**

No vendor lock-in — everything is open-source.

### **⚡ 5. Powered by C++ for true performance**

Unlike Firebase or Supabase, Softadastra Drive uses a custom C++ engine designed for:

- low latency
- high throughput
- minimal memory usage
- efficient concurrency

This backend scales **vertically and horizontally** while keeping predictable performance.

---

# 🏗️ Architecture Overview

```
drive-core (C++)
│
├── HTTP API (REST/JSON)
├── WebSocket Sync Engine
├── File Metadata, Versions, Trees
├── Authentication & Sessions
├── AI Integration Gateway → drive-ai (Python)
└── MySQL Storage
```

Supporting repositories:

- **drive-frontend** → PWA + offline + UI
- **drive-ai** → AI microservice (FastAPI, OCR, summaries, embeddings)
- **sdk-js / sdk-cpp / sdk-python** → Official SDKs
- **docs** → Documentation site

---

# 📦 Project Structure

```
drive-core/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── CHANGELOG.md
├── include/
│   └── softadastra/drive/
│       ├── app/
│       │   └── App.hpp
│       ├── http/
│       ├── service/
│       ├── repository/
│       ├── ws/
│       └── utils/
├── src/
│   ├── main.cpp
│   ├── app/App.cpp
│   ├── http/
│   ├── service/
│   ├── repository/
│   ├── ws/
│   └── utils/
└── tests/
    ├── CMakeLists.txt
    └── *.cpp
```

---

# ⚙️ Build Requirements

- **C++20 compiler**
  - GCC ≥ 11, Clang ≥ 15, MSVC 2022+
- **CMake ≥ 3.20**
- **Vix.cpp installed** (core-only)
- **Boost**
- **OpenSSL**
- **MySQL Connector/C++**
- **Ninja** (recommended)

---

# 🚀 Building & Running

### Using Vix CLI (recommended)

```bash
vix build
vix run
```

### Or manually:

```bash
cmake --preset dev-ninja
cmake --build --preset dev-ninja
./build-ninja/drive-core
```

Visit:

- **http://localhost:8080/**
- **http://localhost:8080/health**

---

# 🔌 API (Early Preview)

### `GET /health`

Returns service health.

### `GET /`

Returns service status.

More routes will follow:

- `/auth/signup`
- `/auth/login`
- `/files/upload`
- `/files/:id`
- `/folders/...`
- `/sync/index`
- `/sync/changes`

---

# 🧪 Tests

Softadastra Drive uses **Catch2 v3** via FetchContent.

Run tests:

```bash
ctest
```

---

# 🧭 Roadmap (Core)

### Phase 1 — Backend Foundation

- [x] Project skeleton
- [ ] Auth system
- [ ] File metadata engine
- [ ] Folder system
- [ ] Sync event model

### Phase 2 — File Engine

- [ ] Upload / download
- [ ] Versions
- [ ] Previews / thumbnails

### Phase 3 — Sync Engine

- [ ] Change cursors
- [ ] Real-time WebSocket sync
- [ ] Conflict resolution

### Phase 4 — AI Engine Gateway

- [ ] Summaries
- [ ] OCR
- [ ] Semantic search

### Phase 5 — SDK ecosystem

- [ ] JS SDK
- [ ] C++ SDK
- [ ] Python SDK

---

# 🪪 License

MIT © Softadastra / Vixcpp authors

---

# 💬 Contributing

Softadastra Drive is open-source and welcomes contributions:

- features
- bug fixes
- documentation
- performance improvements

---

# 🌟 Final Note

Softadastra Drive is more than a storage backend.  
It is the **core building block of the future Softadastra OS**, where apps, AI, sync, identity, and cloud-native workflows merge into a unified ecosystem.

You're not just storing files —  
**you're building the foundation of a global distributed system.**
