import type { ClientEntry, ClientPlatform } from "./types";

/** DeskStream V2 저장소 정보 (README·릴리즈 노트 링크 생성에 사용) */
export const REPOSITORY_URL = "https://github.com/worynim/DeskStream-V2";
export const REPOSITORY_BRANCH = "main";

/** PC 클라이언트 실행 파일을 올리는 저장소 (V1) */
export const CLIENT_RELEASE_REPOSITORY_URL = "https://github.com/worynim/DeskStream";

/** 웹 플래셔 버전 (web/package.json의 version을 빌드 시 주입) */
export const WEB_VERSION: string = __APP_VERSION__;

/** 웹 플래셔 릴리즈 노트 (GitHub) */
export const RELEASE_NOTES_URL = `${REPOSITORY_URL}/blob/${REPOSITORY_BRANCH}/web/RELEASE_NOTES.md`;

/**
 * PC 클라이언트 실행 파일 다운로드 링크 (GitHub Release)
 * PC 클라이언트는 V1과 같은 코드라 V1(DeskStream) 저장소의 Release를 가리킨다.
 * 태그·파일명 규칙은 V1 저장소 .github/workflows/build-clients.yml 과 같다.
 */
export function clientDownloadUrl(client: ClientEntry, platform: ClientPlatform): string {
  const tag = `${client.project}-client-v${client.version}`;
  const ext = platform.startsWith("macos") ? "zip" : "exe";
  const file = `${client.app}-${client.version}-${platform}.${ext}`;
  return `${CLIENT_RELEASE_REPOSITORY_URL}/releases/download/${encodeURIComponent(tag)}/${encodeURIComponent(file)}`;
}

/** 프로젝트별 README 링크 (GitHub) */
export function readmeUrl(projectId: string): string {
  return `${REPOSITORY_URL}/blob/${REPOSITORY_BRANCH}/${encodeURIComponent(projectId)}/README.md`;
}
