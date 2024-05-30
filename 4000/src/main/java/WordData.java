import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;

public class WordData {
        private final String word;
        private final AtomicInteger count;
        private final Set<String> files;

        public WordData(String word) {
            this.word = word;
            this.count = new AtomicInteger(0);
            this.files = new HashSet<>();
        }

        public void incrementCount() {
            count.incrementAndGet();
        }

        public void addFile(String fileName) {
            synchronized (files) {
                files.add(fileName);
            }
        }

        public int getCount() {
            return count.get();
        }

        @Override
        public String toString() {
            return word + " (count: " + count + ", files: " + files + ")";
        }
}
