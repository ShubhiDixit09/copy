/**
 * DHARTI - Google Apps Script Webhook Vault Receiver
 * Root Directory: 'dharti'
 * Automatically creates recursive subfolders (e.g., 'dharti/raw/parivesh/2026/09/08')
 * and stores raw JSON snapshots, PDFs, and evidence files.
 *
 * Deploy as: Web App
 * Execute as: Me (your Google account)
 * Who has access: Anyone (even anonymous)
 */

function doPost(e) {
  try {
    var data = JSON.parse(e.postData.contents);
    var fileName = data.file_name || ("snapshot_" + new Date().getTime() + ".json");
    var mimeType = data.mime_type || "application/json";
    var fileBase64 = data.file_base64 || "";
    var description = data.description || "DHARTI Evidentiary Vault Object";
    var folderPath = data.folder_path || data.folder_name || "dharti/raw/general";

    // Enforce root folder name is 'dharti'
    if (!folderPath.startsWith("dharti")) {
      folderPath = "dharti/" + folderPath;
    }

    // Resolve or create recursive subfolder hierarchy
    var targetFolder = getOrCreateFolderHierarchy(folderPath);

    // Decode base64 and create file
    var decodedBytes = Utilities.base64Decode(fileBase64);
    var blob = Utilities.newBlob(decodedBytes, mimeType, fileName);
    var file = targetFolder.createFile(blob);
    file.setDescription(description);

    var response = {
      status: "SUCCESS",
      file_id: file.getId(),
      file_name: file.getName(),
      folder_path: folderPath,
      web_link: file.getUrl(),
      download_link: "https://drive.google.com/uc?export=download&id=" + file.getId()
    };

    return ContentService.createTextOutput(JSON.stringify(response))
      .setMimeType(ContentService.MimeType.JSON);

  } catch (err) {
    var errResponse = {
      status: "ERROR",
      error: err.toString()
    };
    return ContentService.createTextOutput(JSON.stringify(errResponse))
      .setMimeType(ContentService.MimeType.JSON);
  }
}

/**
 * Recursively creates or traverses folders starting from Drive Root.
 * e.g., 'dharti/raw/parivesh/2026/09/08'
 */
function getOrCreateFolderHierarchy(pathString) {
  var parts = pathString.split("/").filter(function(p) { return p.trim().length > 0; });
  var currentFolder = DriveApp.getRootFolder();

  for (var i = 0; i < parts.length; i++) {
    var name = parts[i].trim();
    var subfolders = currentFolder.getFoldersByName(name);
    if (subfolders.hasNext()) {
      currentFolder = subfolders.next();
    } else {
      currentFolder = currentFolder.createFolder(name);
    }
  }

  return currentFolder;
}
