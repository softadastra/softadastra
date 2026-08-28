(() => {
  "use strict";

  const elements = {
    applications: document.getElementById("applications"),

    message: document.getElementById("message"),

    hostStatus: document.getElementById("host-status"),
    hostDetails: document.getElementById("host"),

    sidebarHostStatus: document.getElementById("sidebar-host-status"),

    metricHostState: document.getElementById("metric-host-state"),
    metricHostDetail: document.getElementById("metric-host-detail"),
    metricApplications: document.getElementById("metric-applications"),
    metricRunning: document.getElementById("metric-running"),

    hostName: document.getElementById("host-name"),
    hostIp: document.getElementById("host-ip"),
    hostConnectivity: document.getElementById("host-connectivity"),
    hostRemote: document.getElementById("host-remote"),

    addOverlay: document.getElementById("add-application-panel"),
    addForm: document.getElementById("add-application-form"),
    selectedProject: document.getElementById("selected-project"),
    applicationConfiguration: document.getElementById(
      "application-configuration",
    ),

    projectName: document.getElementById("project-name"),
    projectDirectory: document.getElementById("project-directory"),

    detailsOverlay: document.getElementById("details-overlay"),
    detailsPanel: document.getElementById("details-panel"),

    logsOverlay: document.getElementById("logs-overlay"),
    logsPanel: document.getElementById("logs-panel"),
    logsTitle: document.getElementById("logs-title"),
    logsStatus: document.getElementById("logs-status"),
    logsOutput: document.getElementById("logs-output"),
    pauseLogs: document.getElementById("pause-logs-button"),
    jumpToLatest: document.getElementById("jump-to-latest-button"),
  };

  const state = {
    applications: [],
    messageTimer: null,

    logs: {
      application: null,
      offset: null,
      paused: false,
      timer: null,
    },
  };

  async function api(path, options = {}) {
    const response = await fetch(path, {
      headers: {
        Accept: "application/json",
        ...options.headers,
      },
      ...options,
    });

    const contentType = response.headers.get("content-type") || "";

    const payload = contentType.includes("application/json")
      ? await response.json()
      : null;

    if (!response.ok) {
      throw new Error(
        payload?.error || "Softadastra could not complete this request.",
      );
    }

    return payload;
  }

  function showMessage(text, kind = "success") {
    elements.message.textContent = text;
    elements.message.className = `message ${kind}`;
    elements.message.hidden = false;

    window.clearTimeout(state.messageTimer);

    state.messageTimer = window.setTimeout(() => {
      elements.message.hidden = true;
    }, 4500);
  }

  function createButton(label, action, className = "button-secondary") {
    const button = document.createElement("button");

    button.type = "button";
    button.textContent = label;
    button.dataset.action = action;
    button.className = `button button-small ${className}`;

    return button;
  }

  function formatProtocol(protocol) {
    return protocol === "ws" ? "WebSocket" : "HTTP";
  }

  function findApplication(name) {
    return state.applications.find((application) => application.name === name);
  }

  function updateMetrics() {
    const running = state.applications.filter(
      (application) => application.state === "running",
    ).length;

    elements.metricApplications.textContent = String(state.applications.length);

    elements.metricRunning.textContent = String(running);
  }

  function createEndpointRow(point = { protocol: "http", port: "" }) {
    const row = document.createElement("div");

    row.className = "endpoint-row";

    const protocol = document.createElement("select");
    protocol.name = "endpoint-protocol";

    const http = document.createElement("option");
    http.value = "http";
    http.textContent = "HTTP";

    const websocket = document.createElement("option");
    websocket.value = "ws";
    websocket.textContent = "WebSocket";

    protocol.append(http, websocket);
    protocol.value = point.protocol || "http";

    const port = document.createElement("input");
    port.name = "endpoint-port";
    port.type = "number";
    port.min = "1";
    port.max = "65535";
    port.inputMode = "numeric";
    port.placeholder = "Port";
    port.value = point.port || "";

    const remove = createButton(
      "Remove",
      "remove-endpoint",
      "button-secondary",
    );

    row.append(protocol, port, remove);

    return row;
  }

  function renderEndpointRows(container, points = []) {
    container.replaceChildren();

    for (const point of points) {
      container.append(createEndpointRow(point));
    }
  }

  function formEndpoints(form) {
    return [...form.querySelectorAll(".endpoint-row")].map((row) => ({
      protocol: row.querySelector("select").value,
      port: row.querySelector("input").value,
    }));
  }

  function createEndpointList(application, withActions = false) {
    const list = document.createElement("div");

    list.className = "endpoint-list";

    const points = application.accesses || [];

    if (!points.length) {
      const empty = document.createElement("span");

      empty.className = "metric-detail";
      empty.textContent = "No access endpoints configured.";

      list.append(empty);

      return list;
    }

    for (const endpoint of points) {
      const row = document.createElement("div");

      row.className = "endpoint";

      const protocol = document.createElement("strong");
      protocol.textContent = formatProtocol(endpoint.protocol);

      const address = document.createElement("span");
      address.textContent = endpoint.url || endpoint.configured;

      row.append(protocol, address);

      if (withActions && endpoint.url) {
        if (endpoint.protocol === "http") {
          const open = document.createElement("a");

          open.href = endpoint.url;
          open.target = "_blank";
          open.rel = "noopener";
          open.textContent = "Open";

          row.append(open);
        } else {
          const copy = createButton(
            "Copy",
            "copy-endpoint",
            "button-secondary",
          );

          copy.dataset.url = endpoint.url;

          row.append(copy);
        }
      }

      list.append(row);
    }

    return list;
  }

  function createApplicationCard(application) {
    const card = document.createElement("article");

    card.className = "application";
    card.dataset.name = application.name;

    const header = document.createElement("div");
    header.className = "application-header";

    const title = document.createElement("h3");
    title.textContent = application.name;

    const status = document.createElement("span");
    status.className = `state ${application.state}`;
    status.textContent = application.state;

    header.append(title, status);

    const actions = document.createElement("div");
    actions.className = "actions";

    const lifecycle =
      application.state === "running"
        ? createButton("Stop", "stop")
        : createButton("Start", "start", "button-primary");

    const logs = createButton("Logs", "logs");

    const details = createButton("Details", "details");

    actions.append(lifecycle, logs, details);

    card.append(header, createEndpointList(application), actions);

    return card;
  }

  function renderApplications() {
    elements.applications.replaceChildren();

    updateMetrics();

    if (!state.applications.length) {
      const empty = document.createElement("div");

      empty.className = "empty";

      const title = document.createElement("strong");
      title.textContent = "No applications yet";

      const description = document.createElement("span");
      description.textContent =
        "Add an existing project and Softadastra will manage it here.";

      const add = createButton("Add application", "add", "button-primary");

      empty.append(title, description, add);

      elements.applications.append(empty);

      return;
    }

    for (const application of state.applications) {
      elements.applications.append(createApplicationCard(application));
    }
  }

  async function loadHost() {
    try {
      const host = await api("/api/host");

      elements.metricHostState.textContent = "Running";
      elements.metricHostDetail.textContent = host.hostname || "This computer";

      elements.sidebarHostStatus.textContent = "Host running";

      elements.hostStatus.textContent = `Network ${host.connectivity} · Remote ${host.remote_status}`;

      elements.hostName.textContent = host.hostname || "Unavailable";

      elements.hostIp.textContent = host.local_ip || "Unavailable";

      elements.hostConnectivity.textContent =
        host.connectivity || "Unavailable";

      elements.hostRemote.textContent = host.remote_status || "Unavailable";
    } catch {
      elements.metricHostState.textContent = "Unavailable";
      elements.metricHostDetail.textContent =
        "Softadastra Host is not responding";

      elements.sidebarHostStatus.textContent = "Host unavailable";

      elements.hostStatus.textContent = "Softadastra Host is unavailable.";

      elements.hostName.textContent = "Unavailable";
      elements.hostIp.textContent = "Unavailable";
      elements.hostConnectivity.textContent = "Unavailable";
      elements.hostRemote.textContent = "Unavailable";
    }
  }

  async function loadApplications() {
    try {
      state.applications = await api("/api/software");

      renderApplications();

      const detailName = elements.detailsPanel.dataset.name;

      if (
        !elements.detailsPanel.hidden &&
        elements.detailsPanel.dataset.mode !== "edit" &&
        detailName
      ) {
        const application = findApplication(detailName);

        if (application) {
          showDetails(application);
        }
      }
    } catch (error) {
      elements.applications.textContent =
        "Softadastra could not load your applications.";

      showMessage(error.message, "error");
    }
  }

  async function refresh() {
    await Promise.all([loadHost(), loadApplications()]);
  }

  async function performApplicationAction(application, action) {
    try {
      await api(
        `/api/software/${encodeURIComponent(application.name)}/${action}`,
        {
          method: "POST",
        },
      );

      await refresh();
    } catch (error) {
      showMessage(error.message, "error");
    }
  }

  function openDetailsPanel() {
    elements.detailsOverlay.hidden = false;
    elements.detailsPanel.hidden = false;
  }

  function closeDetailsPanel() {
    elements.detailsPanel.hidden = true;
    elements.detailsOverlay.hidden = true;

    delete elements.detailsPanel.dataset.name;
    delete elements.detailsPanel.dataset.mode;
  }

  function createPanelHeader(title, description) {
    const header = document.createElement("header");

    header.className = "panel-header";

    const heading = document.createElement("div");

    const eyebrow = document.createElement("p");
    eyebrow.className = "eyebrow";
    eyebrow.textContent = "Application";

    const headingTitle = document.createElement("h2");
    headingTitle.textContent = title;

    const headingDescription = document.createElement("p");
    headingDescription.textContent = description;

    heading.append(eyebrow, headingTitle, headingDescription);

    const close = document.createElement("button");

    close.type = "button";
    close.className = "icon-button";
    close.dataset.action = "close-details";
    close.setAttribute("aria-label", "Close");
    close.textContent = "×";

    header.append(heading, close);

    return header;
  }

  function showDetails(application) {
    openDetailsPanel();

    elements.detailsPanel.dataset.name = application.name;
    elements.detailsPanel.dataset.mode = "details";

    elements.detailsPanel.replaceChildren();

    const header = createPanelHeader(
      application.name,
      "Configuration and runtime information.",
    );

    const body = document.createElement("div");
    body.className = "panel-body";

    const list = document.createElement("dl");
    list.className = "details-list";

    const values = [
      ["Name", application.name],
      ["State", application.state],
      ["Project directory", application.project_directory],
      ["Command", application.command],
    ];

    for (const [label, value] of values) {
      const term = document.createElement("dt");
      const description = document.createElement("dd");

      term.textContent = label;
      description.textContent = value || "-";

      list.append(term, description);
    }

    const accessSection = document.createElement("div");
    accessSection.className = "details-section";

    const accessTitle = document.createElement("h3");
    accessTitle.textContent = "Access endpoints";

    accessSection.append(accessTitle, createEndpointList(application, true));

    const actions = document.createElement("div");
    actions.className = "details-actions";

    actions.append(createButton("Logs", "logs"));

    if (application.state === "stopped") {
      actions.append(createButton("Edit configuration", "edit"));
    }

    actions.append(
      createButton("Remove from Softadastra", "remove", "button-danger"),
    );

    body.append(list, accessSection, actions);

    elements.detailsPanel.append(header, body);
  }

  function showEdit(application) {
    openDetailsPanel();

    elements.detailsPanel.dataset.name = application.name;
    elements.detailsPanel.dataset.mode = "edit";

    elements.detailsPanel.replaceChildren();

    const header = createPanelHeader("Edit configuration", application.name);

    const body = document.createElement("div");
    body.className = "panel-body";

    const form = document.createElement("form");

    const name = document.createElement("label");
    name.className = "field";
    name.innerHTML = "<span>Name</span>";

    const nameInput = document.createElement("input");
    nameInput.name = "name";
    nameInput.required = true;
    nameInput.value = application.name;

    name.append(nameInput);

    const directory = document.createElement("label");
    directory.className = "field";

    const directoryLabel = document.createElement("span");
    directoryLabel.textContent = "Project directory";

    const directoryInput = document.createElement("input");
    directoryInput.name = "project_directory";
    directoryInput.required = true;
    directoryInput.value = application.project_directory;

    directory.append(directoryLabel, directoryInput);

    const command = document.createElement("label");
    command.className = "field";

    const commandLabel = document.createElement("span");
    commandLabel.textContent = "Command";

    const commandInput = document.createElement("input");
    commandInput.name = "command";
    commandInput.required = true;
    commandInput.value = application.command;

    command.append(commandLabel, commandInput);

    const fieldset = document.createElement("fieldset");

    const heading = document.createElement("div");
    heading.className = "fieldset-heading";

    const headingText = document.createElement("div");

    const legend = document.createElement("legend");
    legend.textContent = "Access endpoints";

    const hint = document.createElement("p");
    hint.textContent = "Ports exposed by this application.";

    headingText.append(legend, hint);

    const addEndpoint = createButton("Add endpoint", "add-endpoint");

    heading.append(headingText, addEndpoint);

    const endpoints = document.createElement("div");
    endpoints.className = "endpoints";

    renderEndpointRows(endpoints, application.accesses);

    fieldset.append(heading, endpoints);

    const actions = document.createElement("div");
    actions.className = "form-actions";

    actions.append(createButton("Cancel", "cancel-edit"));

    const save = document.createElement("button");

    save.type = "submit";
    save.className = "button button-primary";
    save.textContent = "Save changes";

    actions.append(save);

    form.append(name, directory, command, fieldset, actions);

    form.addEventListener("click", (event) => {
      const action = event.target.closest("[data-action]");

      if (!action) {
        return;
      }

      if (action.dataset.action === "add-endpoint") {
        endpoints.append(createEndpointRow());
      }

      if (action.dataset.action === "remove-endpoint") {
        action.closest(".endpoint-row")?.remove();
      }

      if (action.dataset.action === "cancel-edit") {
        showDetails(application);
      }
    });

    form.addEventListener("submit", async (event) => {
      event.preventDefault();

      try {
        await api(`/api/software/${encodeURIComponent(application.name)}`, {
          method: "PUT",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify({
            name: nameInput.value,
            project_directory: directoryInput.value,
            command: commandInput.value,
            access_points: formEndpoints(form),
          }),
        });

        showMessage("Configuration saved.");

        closeDetailsPanel();

        await refresh();
      } catch (error) {
        showMessage(error.message, "error");
      }
    });

    body.append(form);

    elements.detailsPanel.append(header, body);
  }

  function openAddPanel() {
    elements.addOverlay.hidden = false;
  }

  function closeAddPanel() {
    elements.addOverlay.hidden = true;
  }

  function resetAddForm() {
    elements.addForm.reset();

    elements.addForm.querySelector(".endpoints").replaceChildren();

    elements.selectedProject.hidden = true;
    elements.applicationConfiguration.hidden = true;
  }

  function applyProject(project) {
    elements.addForm.elements.project_directory.value =
      project.project_directory;

    elements.addForm.elements.name.value = project.name || "";

    elements.addForm.elements.command.value = project.command || "";

    renderEndpointRows(
      elements.addForm.querySelector(".endpoints"),
      project.access_points || [],
    );

    elements.projectName.textContent = project.name || "Selected project";

    elements.projectDirectory.textContent = project.project_directory;

    elements.selectedProject.hidden = false;
    elements.applicationConfiguration.hidden = false;
  }

  async function chooseProject() {
    try {
      const project = await api("/api/project-folder", {
        method: "POST",
      });

      if (!project.cancelled) {
        applyProject(project);
      }
    } catch (error) {
      showMessage(error.message, "error");
    }
  }

  async function addApplication(event) {
    event.preventDefault();

    try {
      await api("/api/software", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          name: elements.addForm.elements.name.value,
          project_directory: elements.addForm.elements.project_directory.value,
          command: elements.addForm.elements.command.value,
          access_points: formEndpoints(elements.addForm),
        }),
      });

      showMessage("Application added.");

      resetAddForm();
      closeAddPanel();

      await refresh();
    } catch (error) {
      showMessage(error.message, "error");
    }
  }

  function stopLogsPolling() {
    if (state.logs.timer) {
      window.clearTimeout(state.logs.timer);
      state.logs.timer = null;
    }
  }

  function closeLogs() {
    stopLogsPolling();

    state.logs.application = null;
    state.logs.offset = null;
    state.logs.paused = false;

    elements.logsPanel.hidden = true;
    elements.logsOverlay.hidden = true;
  }

  function scheduleLogs() {
    stopLogsPolling();

    if (!state.logs.application || state.logs.paused) {
      return;
    }

    state.logs.timer = window.setTimeout(loadLogs, 1000);
  }

  async function loadLogs(reset = false) {
    const application = state.logs.application;

    if (!application || state.logs.paused) {
      return;
    }

    try {
      const path =
        state.logs.offset === null
          ? `/api/software/${encodeURIComponent(application.name)}/logs`
          : `/api/software/${encodeURIComponent(application.name)}/logs?offset=${state.logs.offset}`;

      const result = await api(path);

      if (reset || result.reset) {
        elements.logsOutput.textContent = "";
      }

      if (result.logs) {
        elements.logsOutput.textContent += result.logs;
      }

      state.logs.offset = result.offset;

      elements.logsStatus.textContent = result.running
        ? "Live"
        : "Application stopped";

      const distance =
        elements.logsOutput.scrollHeight -
        elements.logsOutput.scrollTop -
        elements.logsOutput.clientHeight;

      if (distance < 100) {
        elements.logsOutput.scrollTop = elements.logsOutput.scrollHeight;

        elements.jumpToLatest.hidden = true;
      } else {
        elements.jumpToLatest.hidden = false;
      }

      scheduleLogs();
    } catch (error) {
      elements.logsStatus.textContent = "Logs unavailable";

      showMessage(error.message, "error");
    }
  }

  function openLogs(application) {
    stopLogsPolling();

    state.logs.application = application;
    state.logs.offset = null;
    state.logs.paused = false;

    elements.pauseLogs.textContent = "Pause";
    elements.logsTitle.textContent = `${application.name} logs`;

    elements.logsStatus.textContent = "Live";
    elements.logsOutput.textContent = "Loading logs...";

    elements.logsOverlay.hidden = false;
    elements.logsPanel.hidden = false;

    loadLogs(true);
  }

  elements.applications.addEventListener("click", async (event) => {
    const action = event.target.closest("[data-action]");

    if (!action) {
      return;
    }

    if (action.dataset.action === "add") {
      openAddPanel();
      return;
    }

    const card = action.closest(".application");

    if (!card) {
      return;
    }

    const application = findApplication(card.dataset.name);

    if (!application) {
      return;
    }

    if (action.dataset.action === "details") {
      showDetails(application);
      return;
    }

    if (action.dataset.action === "logs") {
      openLogs(application);
      return;
    }

    await performApplicationAction(application, action.dataset.action);
  });

  elements.detailsPanel.addEventListener("click", async (event) => {
    const action = event.target.closest("[data-action]");

    if (!action) {
      return;
    }

    if (action.dataset.action === "close-details") {
      closeDetailsPanel();
      return;
    }

    const application = findApplication(elements.detailsPanel.dataset.name);

    if (!application) {
      return;
    }

    if (action.dataset.action === "edit") {
      showEdit(application);
      return;
    }

    if (action.dataset.action === "logs") {
      closeDetailsPanel();
      openLogs(application);
      return;
    }

    if (action.dataset.action === "copy-endpoint") {
      await navigator.clipboard.writeText(action.dataset.url);

      showMessage("WebSocket address copied.");

      return;
    }

    if (action.dataset.action === "remove") {
      const confirmed = window.confirm(
        `Remove ${application.name} from Softadastra?\n\nIts files will not be deleted.`,
      );

      if (!confirmed) {
        return;
      }

      try {
        await api(`/api/software/${encodeURIComponent(application.name)}`, {
          method: "DELETE",
        });

        closeDetailsPanel();

        showMessage("Application removed.");

        await refresh();
      } catch (error) {
        showMessage(error.message, "error");
      }
    }
  });

  elements.addForm.addEventListener("click", (event) => {
    const action = event.target.closest("[data-action]");

    if (action?.dataset.action === "remove-endpoint") {
      action.closest(".endpoint-row")?.remove();
    }
  });

  elements.addForm.addEventListener("submit", addApplication);

  document
    .getElementById("choose-project-folder-button")
    .addEventListener("click", chooseProject);

  document
    .getElementById("add-endpoint-button")
    .addEventListener("click", () => {
      elements.addForm.querySelector(".endpoints").append(createEndpointRow());
    });

  document
    .getElementById("add-application-button")
    .addEventListener("click", openAddPanel);

  document
    .getElementById("close-add-button")
    .addEventListener("click", closeAddPanel);

  document
    .getElementById("cancel-add-button")
    .addEventListener("click", closeAddPanel);

  document
    .getElementById("host-details-button")
    .addEventListener("click", () => {
      elements.hostDetails.hidden = !elements.hostDetails.hidden;
    });

  document.getElementById("refresh-button").addEventListener("click", refresh);

  document
    .getElementById("close-logs-button")
    .addEventListener("click", closeLogs);

  elements.pauseLogs.addEventListener("click", () => {
    state.logs.paused = !state.logs.paused;

    elements.pauseLogs.textContent = state.logs.paused ? "Resume" : "Pause";

    elements.logsStatus.textContent = state.logs.paused ? "Paused" : "Live";

    if (state.logs.paused) {
      stopLogsPolling();
    } else {
      loadLogs();
    }
  });

  document
    .getElementById("clear-logs-button")
    .addEventListener("click", async () => {
      if (!state.logs.application) {
        return;
      }

      try {
        await api(
          `/api/software/${encodeURIComponent(state.logs.application.name)}/logs/clear`,
          {
            method: "POST",
          },
        );

        state.logs.offset = 0;
        elements.logsOutput.textContent = "";

        showMessage("Logs cleared.");

        if (!state.logs.paused) {
          loadLogs(true);
        }
      } catch (error) {
        showMessage(error.message, "error");
      }
    });

  elements.jumpToLatest.addEventListener("click", () => {
    elements.logsOutput.scrollTop = elements.logsOutput.scrollHeight;

    elements.jumpToLatest.hidden = true;
  });

  elements.logsOutput.addEventListener("scroll", () => {
    const distance =
      elements.logsOutput.scrollHeight -
      elements.logsOutput.scrollTop -
      elements.logsOutput.clientHeight;

    elements.jumpToLatest.hidden = distance < 100;
  });

  elements.addOverlay.addEventListener("click", (event) => {
    if (event.target === elements.addOverlay) {
      closeAddPanel();
    }
  });

  elements.detailsOverlay.addEventListener("click", (event) => {
    if (event.target === elements.detailsOverlay) {
      closeDetailsPanel();
    }
  });

  elements.logsOverlay.addEventListener("click", (event) => {
    if (event.target === elements.logsOverlay) {
      closeLogs();
    }
  });

  refresh();

  window.setInterval(refresh, 5000);
})();
